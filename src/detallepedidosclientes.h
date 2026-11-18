#ifndef DETALLEPEDIDOSCLIENTES_H
#define DETALLEPEDIDOSCLIENTES_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Detallepedidosclientes;
}

class Detallepedidosclientes : public QDialog
{
    Q_OBJECT

public:
    explicit Detallepedidosclientes(QWidget *parent = nullptr);
    ~Detallepedidosclientes();

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

    void on_txtCantidad_textChanged(const QString &arg1);
    void on_txtPrecioUnitario_textChanged(const QString &arg1);

private:
    Ui::Detallepedidosclientes *ui;
    QSqlQueryModel *modelo;
};

#endif // DETALLEPEDIDOSCLIENTES_H
