#ifndef PEDIDOSCLIENTES_H
#define PEDIDOSCLIENTES_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Pedidosclientes;
}

class Pedidosclientes : public QDialog
{
    Q_OBJECT

public:
    explicit Pedidosclientes(QWidget *parent = nullptr);
    ~Pedidosclientes();

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
    Ui::Pedidosclientes *ui;
    QSqlQueryModel *modelo;
};

#endif // PEDIDOSCLIENTES_H
