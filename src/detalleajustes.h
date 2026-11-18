#ifndef DETALLEAJUSTES_H
#define DETALLEAJUSTES_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Detalleajustes;
}

class Detalleajustes : public QDialog
{
    Q_OBJECT

public:
    explicit Detalleajustes(QWidget *parent = nullptr);
    ~Detalleajustes();

private slots:
    void cargarCombos();
    void cargarDatos();
    void limpiarCampos();
    void calcularDiferencia();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous = QModelIndex());

    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnSalir_clicked();

private:
    Ui::Detalleajustes *ui;
    QSqlQueryModel *modelo;
};

#endif // DETALLEAJUSTES_H
