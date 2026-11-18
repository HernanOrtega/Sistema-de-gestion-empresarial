#ifndef COBROTIPO_H
#define COBROTIPO_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Cobrotipo;  // Todo junto, sin espacio
}

class Cobrotipo : public QDialog  // Todo junto, sin espacio
{
    Q_OBJECT

public:
    explicit Cobrotipo(QWidget *parent = nullptr);  // Todo junto
    ~Cobrotipo();  // Todo junto

private slots:
    void cargarDatos();
    void limpiarCampos();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous = QModelIndex());

    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnSalir_clicked();

private:
    Ui::Cobrotipo *ui;  // Todo junto
    QSqlQueryModel *modelo;
};

#endif // COBROTIPO_H
