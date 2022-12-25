#ifndef MTKPROG_H
#define MTKPROG_H
#include <QObject>


class mtkprog : public QObject
{
    Q_OBJECT

public:
    mtkprog();

public slots:
    void Start(void);

};

#endif // MTKPROG_H
