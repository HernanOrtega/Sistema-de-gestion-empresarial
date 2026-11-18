#ifndef COBROS_H
#define COBROS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Cobros;
}

class Cobros : public QDialog
{
    Q_OBJECT

public:
    explicit Cobros(QWidget *parent = nullptr);
    ~Cobros();

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
    Ui::Cobros *ui;
    QSqlQueryModel *modelo;
};

#endif // COBROS_H
