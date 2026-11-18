#ifndef DETALLEPEDIDOSPROVEEDORES_H
#define DETALLEPEDIDOSPROVEEDORES_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Detallepedidosproveedores;
}

class Detallepedidosproveedores : public QDialog
{
    Q_OBJECT

public:
    explicit Detallepedidosproveedores(QWidget *parent = nullptr);
    ~Detallepedidosproveedores();

private slots:
    void cargarCombos();
    void cargarDatos();
    void limpiarCampos();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous = QModelIndex());
    void calcularSubtotal();

    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnSalir_clicked();

private:
    Ui::Detallepedidosproveedores *ui;
    QSqlQueryModel *modelo;
};

#endif // DETALLEPEDIDOSPROVEEDORES_H
