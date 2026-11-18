#ifndef DETALLECOMPRAS_H
#define DETALLECOMPRAS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Detallecompras;
}

class Detallecompras : public QDialog
{
    Q_OBJECT

public:
    explicit Detallecompras(QWidget *parent = nullptr);
    ~Detallecompras();

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
    Ui::Detallecompras *ui;
    QSqlQueryModel *modelo;
};

#endif // DETALLECOMPRAS_H
