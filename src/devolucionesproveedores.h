#ifndef DEVOLUCIONESPROVEEDORES_H
#define DEVOLUCIONESPROVEEDORES_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Devolucionesproveedores;
}

class Devolucionesproveedores : public QDialog
{
    Q_OBJECT

public:
    explicit Devolucionesproveedores(QWidget *parent = nullptr);
    ~Devolucionesproveedores();

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
    Ui::Devolucionesproveedores *ui;
    QSqlQueryModel *modelo;
};

#endif // DEVOLUCIONESPROVEEDORES_H
