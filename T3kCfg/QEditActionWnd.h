#ifndef QEDITACTIONWND_H
#define QEDITACTIONWND_H

#include <QDialog>
#include "QLangManager.h"

namespace Ui{
    class QEditActionWnd;
}

class T3kHandle;
class QMouseMappingTable;
class QEditActionWnd : public QDialog, public QLangManager::LangChangeNotify
{
    Q_OBJECT

public:
    explicit QEditActionWnd(T3kHandle*& pHandle, QWidget *parent = 0);
    ~QEditActionWnd();

    enum ProfileType { TypeSingle, TypeDouble, TypeMove, TypeError };
    enum EditMode { ModeNone, ModeUserDefined, ModeFunctionKey };

    void SetProfileInfo( int nProfileIndex, uchar cKey, ushort wKeyValue, bool bTaskSwitch );
    virtual void OnChangeLanguage();

protected:
    void Init();
    void OnKeyEditChange( ushort wKeyValue );

    void SetPredefineProfileTypes( ProfileType Type, bool bTaskSwitch );
    void SetEditMode( EditMode Mode, ushort wKeyValue );

    virtual void showEvent(QShowEvent *evt);
    virtual void hideEvent(QHideEvent *evt);

protected:
    int			m_nProfileIndex;
    uchar		m_cProfileKey;
    ushort		m_wProfileValue;

private:
    Ui::QEditActionWnd* ui;
    T3kHandle*&        m_pT3kHandle;
    QMouseMappingTable* m_pTableWnd;

private slots:
    void on_CBAction_activated(int index);
    void on_CBAction_currentIndexChanged(int index);
    void onFunctionKeyActivated(int index);
    void on_BtnApply_clicked();
    void onKeyPress_Signal(ushort nValue);
};

#endif // QEDITACTIONWND_H
