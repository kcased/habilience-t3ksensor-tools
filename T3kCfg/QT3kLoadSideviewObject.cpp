#include "QT3kLoadSideviewObject.h"

#include "QT3kUserData.h"
#include "CfgCustomCmdDef.h"

#include <QPainter>
#include <QTimerEvent>

#define SIDEVIEW_IMG_WIDTH  199
#define SIDEVIEW_IMG_HEIGHT 49

#define LD_Cam1             0
#define LD_Cam2             1
#define LD_Cam3             2
#define LD_Cam4             3
#define LD_End              4

QT3kLoadSideviewObject::QT3kLoadSideviewObject(T3kHandle*& pHandle, QObject *parent) :
    QT3kHIDObject(pHandle, parent)
{
    m_nLoadCamNo = LD_End;

    m_pSideViewImage = NULL;
    m_nCountAskCam = 0;

    m_pStorageHandle = NULL;

    connect( this, SIGNAL(CheckTimeOut()), this, SLOT(on_CheckTimeOut()) );
}

QT3kLoadSideviewObject::~QT3kLoadSideviewObject()
{
    if( m_pSideViewImage )
    {
        delete m_pSideViewImage;
        m_pSideViewImage = NULL;
    }
}

bool QT3kLoadSideviewObject::Start(SensorLogData *pStorage)
{
    if( m_bStart ) return false;
    if( !pStorage ) return false;

    m_bStart = true;
    m_pStorageHandle = pStorage;

    int nModel = QT3kUserData::GetInstance()->GetModel();
    m_nCountAskCam = nModel == 0x0000 || nModel == 0x3000 || nModel == 0x3100 ? 2 : 4;

    m_nLoadCamNo = LD_Cam1;
    m_strSendCmd = "cam1/mode=sideview";
    m_nSendCmdID = m_pT3kHandle->SendCommand( (const char*)m_strSendCmd.toUtf8().data(), true );
    m_pT3kHandle->SetReportView( true );

    StartAsyncTimeoutChecker( 10000 );

    return true;
}

void QT3kLoadSideviewObject::NextSideView(int nCamNo)
{
    m_nLoadCamNo = nCamNo;
    int nSub = (int)(nCamNo/2);
    m_strSendCmd = QString("cam%1/").arg(nCamNo%2+1);
    for( int i=0; i<nSub; i++ )
        m_strSendCmd += "sub/";

    m_strSendCmd += "mode=sideview";
    m_nSendCmdID = m_pT3kHandle->SendCommand( (const char*)m_strSendCmd.toUtf8().data(), true );
}

void QT3kLoadSideviewObject::FinishSideView()
{
    StopAsyncTimeoutChecker();

    m_pT3kHandle->SendCommand( (const char*)"cam1/mode=detection", true );
    m_pT3kHandle->SendCommand( (const char*)"cam2/mode=detection", true );

    m_pT3kHandle->SetReportView( false );

    QMap<int, CMLogDataGroup*>::iterator iterCM1Sub = m_pStorageHandle->CM.find( CM1_1 );
    QMap<int, CMLogDataGroup*>::iterator iterCM2Sub = m_pStorageHandle->CM.find( CM2_1 );
    if( iterCM1Sub != m_pStorageHandle->CM.end() && iterCM2Sub != m_pStorageHandle->CM.end() )
    {
        if( iterCM1Sub.value() && iterCM2Sub.value() && iterCM1Sub.value()->bNoCam && iterCM2Sub.value()->bNoCam )
        {
            m_pStorageHandle->CM.erase( iterCM1Sub );
            m_pStorageHandle->CM.erase( iterCM2Sub );
        }
    }

    for( QMap<int,CMLogDataGroup*>::iterator iter = m_pStorageHandle->CM.begin(); iter != m_pStorageHandle->CM.end(); ++iter )
    {
        if( !iter.value() || !iter.value()->bNoCam ) continue;

        int nPart = iter.key();
        if( !m_pSideViewImage )
            m_pSideViewImage = new QImage( SIDEVIEW_IMG_WIDTH, SIDEVIEW_IMG_HEIGHT, QImage::Format_RGB32 );
        else
            (*m_pSideViewImage) = QImage( SIDEVIEW_IMG_WIDTH, SIDEVIEW_IMG_HEIGHT, QImage::Format_RGB32 );

        FillEmptySideview( m_pSideViewImage, SIDEVIEW_IMG_WIDTH, SIDEVIEW_IMG_HEIGHT );
        m_pSideViewImage->save( m_strSavePath + QString("/cam%1.png").arg(nPart), "PNG" );
    }

    QString strLog( "[" + QLangManager::GetPtr()->GetResource().GetResString(QString::fromUtf8("SENSOR DIAGNOSIS"), QString::fromUtf8("LOG_COMPLETE")) + "] " );
    strLog += QLangManager::GetPtr()->GetResource().GetResString(QString::fromUtf8("SENSOR DIAGNOSIS"), QString::fromUtf8("LOG_SIDEVIEW"));
    emit PrintProgreeLog( strLog );

    emit Complete( LDS_SIDEVIEW );
    m_bStart = false;
}

void QT3kLoadSideviewObject::OnPRV(ResponsePart Part, ushort, const char *, int nWidth, int nHeight, int, unsigned char *pImgBuffer)
{
    int nCamNo = Part-1;
    Q_ASSERT( nCamNo >= 0 && nCamNo < m_nCountAskCam );

    if( !m_pSideViewImage )
        m_pSideViewImage = new QImage( pImgBuffer, nWidth, nHeight, QImage::Format_RGB32 );
    else
        (*m_pSideViewImage) = QImage( pImgBuffer, nWidth, nHeight, QImage::Format_RGB32 );
    (*m_pSideViewImage) = m_pSideViewImage->mirrored();

    QString strPngName;
    int nPart = nCamNo%2;
    int nSub = nCamNo/2;
    strPngName = QString("/cam%1").arg(nPart+1);
    for( int i=0; i<nSub; i++ )
        strPngName += QString("-%1").arg(nSub);
    //strPngName = QString("/cam%1").arg(nCamNo+1);

    m_pSideViewImage->save( m_strSavePath + strPngName + ".png", "PNG" );

    CMLogDataGroup* pCM = m_pStorageHandle->CM.value( Part );
    if( !pCM )
        pCM = new CMLogDataGroup;

    pCM->bNoCam = false;
    m_pStorageHandle->CM.insert( Part, pCM );

    QString strLog( "[" + QLangManager::GetPtr()->GetResource().GetResString(QString::fromUtf8("SENSOR DIAGNOSIS"), QString::fromUtf8("LOG_OK")) + "] " );
    strLog += QLangManager::GetPtr()->GetResource().GetResString(QString::fromUtf8("SENSOR DIAGNOSIS"), QString::fromUtf8("LOG_SIDEVIEW"));

    switch(nCamNo)
    {
    case 0:
        strLog += QString(" - %1").arg( QLangManager::GetPtr()->GetResource().GetResString(
                QString::fromUtf8("SENSOR DIAGNOSIS"), QString::fromUtf8("TITLE_SENSOR1")) );
        break;
    case 1:
        strLog += QString(" - %1").arg( QLangManager::GetPtr()->GetResource().GetResString(
                QString::fromUtf8("SENSOR DIAGNOSIS"), QString::fromUtf8("TITLE_SENSOR2")) );
        break;
    case 2:
        strLog += QString(" - %1").arg( QLangManager::GetPtr()->GetResource().GetResString(
                QString::fromUtf8("SENSOR DIAGNOSIS"), QString::fromUtf8("TITLE_SENSOR1_1")) );
        break;
    case 3:
        strLog += QString(" - %1").arg( QLangManager::GetPtr()->GetResource().GetResString(
                QString::fromUtf8("SENSOR DIAGNOSIS"), QString::fromUtf8("TITLE_SENSOR2_1")) );
        break;
    default:
        break;
    }

    emit PrintProgreeLog( strLog );

    if( nCamNo+1 < m_nCountAskCam )
    {
        NextSideView( nCamNo+1 );
        return;
    }

    FinishSideView();
}

void QT3kLoadSideviewObject::OnRSE(ResponsePart Part, ushort, const char *sPartId, long lId, bool, const char *sCmd)
{
    if( m_nSendCmdID != lId ) return;

    QString strPartID( sPartId );
    if( !strPartID.contains( "RSE" ) ) return;

    if( strstr( sCmd, cstrNoCam ) == sCmd )
    {
        if( m_strSendCmd.contains("sub/") && Part < CM1_1 )
            Part = (ResponsePart)(Part+2);
        if( Part != (ResponsePart)(m_nLoadCamNo+1) ) return;

        CMLogDataGroup* pCM = m_pStorageHandle->CM.value( Part );
        if( !pCM )
            pCM = new CMLogDataGroup;

        pCM->bNoCam = true;
        m_pStorageHandle->CM.insert( Part, pCM );

        m_nLoadCamNo++;
        if( m_nLoadCamNo < m_nCountAskCam )
        {
            NextSideView( m_nLoadCamNo );
            return;
        }

        FinishSideView();
    }
}

void QT3kLoadSideviewObject::FillEmptySideview(QImage *pImg, int nWidth, int nHeight)
{
    if( !pImg )
        pImg = new QImage( nWidth, nHeight, QImage::Format_RGB32 );
    else
        (*pImg) = QImage( nWidth, nHeight, QImage::Format_RGB32 );

    QRect rcClient( 0, 0, nWidth, nHeight );
    QPainter dc;
    dc.begin( pImg );

    dc.fillRect( rcClient, Qt::black );
    dc.setPen( Qt::white );
    QString str( QLangManager::GetPtr()->GetResource().GetResString(QString::fromUtf8("SENSOR DIAGNOSIS SIDEVIEW"), QString::fromUtf8("TEXT_NO_IMAGE")) );
    dc.drawText( rcClient, str, QTextOption( Qt::AlignVCenter | Qt::AlignHCenter ) );

    dc.end();
}

void QT3kLoadSideviewObject::on_CheckTimeOut()
{
    m_nLoadCamNo++;
    if( m_nLoadCamNo < m_nCountAskCam )
    {
        NextSideView( m_nLoadCamNo );
        return;
    }

    FinishSideView();
}
