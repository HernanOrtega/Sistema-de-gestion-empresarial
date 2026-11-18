#ifndef DETALLEORDENESCOMPRA_H
#define DETALLEORDENESCOMPRA_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Detalleordenescompra;
}

class Detalleordenescompra : public QDialog
{
    Q_OBJECT

public:
    explicit Detalleordenescompra(QWidget *parent = nullptr);
    ~Detalleordenescompra();

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
    Ui::Detalleordenescompra *ui;
    QSqlQueryModel *modelo;
};

#endif // DETALLEORDENESCOMPRA_H
