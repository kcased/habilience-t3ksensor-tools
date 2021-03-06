#ifndef QICONTOOLBUTTON_H
#define QICONTOOLBUTTON_H

#include <QToolButton>

class QIconToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit QIconToolButton(QWidget *parent = 0);

    void ChangeIcon( QString strFilePahtName );

    void setIcon(const QIcon &icon);

    void setRaiseAction( bool bAction );

protected:
    virtual void paintEvent(QPaintEvent *);

protected:
    QImage              m_IconImage;
    bool                m_bAutoRaise;

signals:

public slots:

};

#endif // QICONTOOLBUTTON_H
