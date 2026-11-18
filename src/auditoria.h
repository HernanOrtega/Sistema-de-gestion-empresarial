#ifndef AUDITORIA_H
#define AUDITORIA_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Auditoria;
}

class Auditoria : public QDialog
{
    Q_OBJECT

public:
    explicit Auditoria(QWidget *parent = nullptr);
    ~Auditoria();

private slots:
    void cargarDatos();
    void limpiarFiltros();
    void aplicarFiltros();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous = QModelIndex());

    void on_btnLimpiarFiltros_clicked();
    void on_btnAplicarFiltros_clicked();
    void on_btnSalir_clicked();

private:
    Ui::Auditoria *ui;
    QSqlQueryModel *modelo;
};

#endif // AUDITORIA_H
