#ifndef PRESUPUESTOS_H
#define PRESUPUESTOS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Presupuestos;
}

class Presupuestos : public QDialog
{
    Q_OBJECT

public:
    explicit Presupuestos(QWidget *parent = nullptr);
    ~Presupuestos();

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
    Ui::Presupuestos *ui;
    QSqlQueryModel *modelo;
};

#endif // PRESUPUESTOS_H
