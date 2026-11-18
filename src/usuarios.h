#ifndef USUARIOS_H
#define USUARIOS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Usuarios;
}

class Usuarios : public QDialog
{
    Q_OBJECT

public:
    explicit Usuarios(QWidget *parent = nullptr);
    ~Usuarios();

private slots:
    void cargarEmpleados();
    void cargarDatos();
    void limpiarCampos();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous = QModelIndex());

    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnSalir_clicked();

private:
    Ui::Usuarios *ui;
    QSqlQueryModel *modelo;
};

#endif // USUARIOS_H
