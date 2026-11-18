#ifndef DETALLECOBROS_H
#define DETALLECOBROS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Detallecobros;
}

class Detallecobros : public QDialog
{
    Q_OBJECT

public:
    explicit Detallecobros(QWidget *parent = nullptr);
    ~Detallecobros();

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
    Ui::Detallecobros *ui;
    QSqlQueryModel *modelo;
};

#endif // DETALLECOBROS_H
