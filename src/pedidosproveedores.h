#ifndef PEDIDOSPROVEEDORES_H
#define PEDIDOSPROVEEDORES_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Pedidosproveedores;
}

class Pedidosproveedores : public QDialog
{
    Q_OBJECT

public:
    explicit Pedidosproveedores(QWidget *parent = nullptr);
    ~Pedidosproveedores();

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
    Ui::Pedidosproveedores *ui;
    QSqlQueryModel *modelo;
};

#endif // PEDIDOSPROVEEDORES_H
