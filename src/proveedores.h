#ifndef PROVEEDORES_H
#define PROVEEDORES_H

#include <QDialog>
#include <QSqlQueryModel>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui {
class Proveedores;
}
QT_END_NAMESPACE

class Proveedores : public QDialog
{
    Q_OBJECT

public:
    explicit Proveedores(QWidget *parent = nullptr);
    ~Proveedores();

private slots:
    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnCerrar_clicked();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous);
    void on_tablaProveedores_clicked(const QModelIndex &index);

private:
    Ui::Proveedores *ui;
    QSqlQueryModel *modelo;

    void cargarCiudades();
    void cargarDatos();
    void limpiarCampos();
};

#endif // PROVEEDORES_H


