#ifndef COTIZACIONES_H
#define COTIZACIONES_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Cotizaciones;
}

class Cotizaciones : public QDialog
{
    Q_OBJECT

public:
    explicit Cotizaciones(QWidget *parent = nullptr);
    ~Cotizaciones();

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
    Ui::Cotizaciones *ui;
    QSqlQueryModel *modelo;
};

#endif // COTIZACIONES_H
