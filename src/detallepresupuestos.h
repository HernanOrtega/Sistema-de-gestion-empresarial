#ifndef DETALLEPRESUPUESTOS_H
#define DETALLEPRESUPUESTOS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Detallepresupuestos;
}

class Detallepresupuestos : public QDialog
{
    Q_OBJECT

public:
    explicit Detallepresupuestos(QWidget *parent = nullptr);
    ~Detallepresupuestos();

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
    Ui::Detallepresupuestos *ui;
    QSqlQueryModel *modelo;
};

#endif // DETALLEPRESUPUESTOS_H
