#ifndef PAGOS_H
#define PAGOS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Pagos;
}

class Pagos : public QDialog
{
    Q_OBJECT

public:
    explicit Pagos(QWidget *parent = nullptr);
    ~Pagos();

private slots:
    void cargarCombos();
    void cargarDatos();
    void limpiarCampos();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous = QModelIndex());

    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnSalir_clicked();

private:
    Ui::Pagos *ui;
    QSqlQueryModel *modelo;
};

#endif // PAGOS_H
