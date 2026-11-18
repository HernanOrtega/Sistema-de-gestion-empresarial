#ifndef CUENTASPAGAR_H
#define CUENTASPAGAR_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Cuentaspagar;
}

class Cuentaspagar : public QDialog
{
    Q_OBJECT

public:
    explicit Cuentaspagar(QWidget *parent = nullptr);
    ~Cuentaspagar();

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
    Ui::Cuentaspagar *ui;
    QSqlQueryModel *modelo;
};

#endif // CUENTASPAGAR_H
