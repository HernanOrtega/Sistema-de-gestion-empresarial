#ifndef PRODUCTOS_H
#define PRODUCTOS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Productos;
}

class Productos : public QDialog
{
    Q_OBJECT

public:
    explicit Productos(QWidget *parent = nullptr);
    ~Productos();

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
    Ui::Productos *ui;
    QSqlQueryModel *modelo;
};

#endif // PRODUCTOS_H
