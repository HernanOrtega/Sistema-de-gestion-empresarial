#ifndef MONEDAS_H
#define MONEDAS_H

#include <QDialog>
#include <QSqlTableModel>

namespace Ui {
class Monedas;
}

class Monedas : public QDialog
{
    Q_OBJECT

public:
    explicit Monedas(QWidget *parent = nullptr);
    ~Monedas();

private slots:
    void cargarDatos();
    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnCerrar_clicked();
    void on_tablaMonedas_clicked(const QModelIndex &index);
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous);

private:
    Ui::Monedas *ui;
    QSqlTableModel *modelo;
    void limpiarCampos();
};

#endif // MONEDAS_H

