#ifndef DETALLEVENTAS_H
#define DETALLEVENTAS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Detalleventas;
}

class Detalleventas : public QDialog
{
    Q_OBJECT

public:
    explicit Detalleventas(QWidget *parent = nullptr);
    ~Detalleventas();

private slots:
    void cargarCombos();
    void cargarDatos();
    void limpiarCampos();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous = QModelIndex());
    void calcularSubtotal(); // AÑADIR ESTA LÍNEA

    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnSalir_clicked();

private:
    Ui::Detalleventas *ui;
    QSqlQueryModel *modelo;
};

#endif // DETALLEVENTAS_H
