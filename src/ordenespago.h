#ifndef ORDENESPAGO_H
#define ORDENESPAGO_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Ordenespago;
}

class Ordenespago : public QDialog
{
    Q_OBJECT

public:
    explicit Ordenespago(QWidget *parent = nullptr);
    ~Ordenespago();

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
    Ui::Ordenespago *ui;
    QSqlQueryModel *modelo;
};

#endif // ORDENESPAGO_H
