#ifndef DETALLEORDENESPAGO_H
#define DETALLEORDENESPAGO_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Detalleordenespago;
}

class Detalleordenespago : public QDialog
{
    Q_OBJECT

public:
    explicit Detalleordenespago(QWidget *parent = nullptr);
    ~Detalleordenespago();

private slots:
    void cargarCombos();
    void cargarDatos();
    void limpiarCampos();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous = QModelIndex());
    void calcularSaldoActual();

    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnSalir_clicked();

private:
    Ui::Detalleordenespago *ui;
    QSqlQueryModel *modelo;
};

#endif // DETALLEORDENESPAGO_H
