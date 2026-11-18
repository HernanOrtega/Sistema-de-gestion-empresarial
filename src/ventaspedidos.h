#ifndef VENTASPEDIDOS_H
#define VENTASPEDIDOS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Ventaspedidos;
}

class Ventaspedidos : public QDialog
{
    Q_OBJECT

public:
    explicit Ventaspedidos(QWidget *parent = nullptr);
    ~Ventaspedidos();

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
    Ui::Ventaspedidos *ui;
    QSqlQueryModel *modelo;
};

#endif // VENTASPEDIDOS_H
