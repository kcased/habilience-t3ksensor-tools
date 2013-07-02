#ifndef QEDITACTION2WDWND_H
#define QEDITACTION2WDWND_H

#include <QDialog>

#include "QLangManager.h"


namespace Ui{
    class QEditAction2WDWnd;
}

class T3kHandle;
class QMouseMappingTable;
class QComboBox;
class QLabel;

class QEditAction2WDWnd : public QDialog, public QLangManager::LangChangeNotify
{
    Q_OBJECT

public:
    explicit QEditAction2WDWnd(T3kHandle*& pHandle, QWidget *parent = 0);
    ~QEditAction2WDWnd();

    enum EditMode { ModeNone, ModeUserDefined, ModeFunctionKey };

    void SetProfileInfo( int nProfileIndex, uchar cKey, ushort wKeyValue1, ushort wKeyValue2 );

    virtual void OnChangeLanguage();
protected:
    void Init();
    void OnKeyEditChange(ushort wKeyValue);

    void SetEditMode( EditMode Mode, ushort wKeyValue );
    void SetActionWithProfileInfo( QComboBox* pWndComboAction, QLabel* pDetailLable, ushort wKeyValue );
    void SetPredefineProfileTypes( QComboBox* pWndComboAction );

    void OnCbnSelchangeComboAction( QComboBox* pWndComboAction );

protected:
    virtual void showEvent(QShowEvent *evt);
    virtual void hideEvent(QHideEvent *evt);

private:
    Ui::QEditAction2WDWnd* ui;
    T3kHandle*&            m_pT3kHandle;

    QMouseMappingTable*     m_pTableWnd;
    QComboBox*              m_pWndActiveComboAction;

    int                     m_nProfileIndex;
    uchar                   m_cProfileKey;
    ushort                  m_wProfileValue[2];

    int                     m_nLoad;

private slots:
    void on_BtnChkLink_toggled(bool checked);
    void onFunctionKeyActivated(int index);
    void on_BtnApply_clicked();
    void on_CBActionZoomIn_activated(int index);
    void on_CBActionZoomOut_activated(int index);
    void onKeyPress_Signal(ushort nValue);
};

#endif // QEDITACTION2WDWND_H
