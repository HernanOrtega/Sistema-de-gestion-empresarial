#ifndef CUENTASCOBRAR_H
#define CUENTASCOBRAR_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Cuentascobrar;
}

class Cuentascobrar : public QDialog
{
    Q_OBJECT

public:
    explicit Cuentascobrar(QWidget *parent = nullptr);
    ~Cuentascobrar();

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
    Ui::Cuentascobrar *ui;
    QSqlQueryModel *modelo;
};

#endif // CUENTASCOBRAR_H
