#ifndef DETALLEDEVOLUCIONESPROVEEDORES_H
#define DETALLEDEVOLUCIONESPROVEEDORES_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Detalledevolucionesproveedores;
}

class Detalledevolucionesproveedores : public QDialog
{
    Q_OBJECT

public:
    explicit Detalledevolucionesproveedores(QWidget *parent = nullptr);
    ~Detalledevolucionesproveedores();

private slots:
    void cargarCombos();
    void cargarDatos();
    void limpiarCampos();
    void calcularSubtotal();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous = QModelIndex());

    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnSalir_clicked();

private:
    Ui::Detalledevolucionesproveedores *ui;
    QSqlQueryModel *modelo;
};

#endif // DETALLEDEVOLUCIONESPROVEEDORES_H
