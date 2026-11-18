#ifndef CLIENTES_H
#define CLIENTES_H

#include <QDialog>
#include <QSqlQueryModel>
#include <QDate>

QT_BEGIN_NAMESPACE
namespace Ui {
class Clientes;
}
QT_END_NAMESPACE

class Clientes : public QDialog
{
    Q_OBJECT

public:
    explicit Clientes(QWidget *parent = nullptr);
    ~Clientes();

private slots:
    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnCerrar_clicked();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous);
    void on_tablaClientes_clicked(const QModelIndex &index);

private:
    Ui::Clientes *ui;
    QSqlQueryModel *modelo;

    void cargarCiudades();
    void cargarDatos();
    void limpiarCampos();
};

#endif // CLIENTES_H
