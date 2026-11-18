#ifndef TIPOAJUSTES_H
#define TIPOAJUSTES_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Tipoajustes;
}

class Tipoajustes : public QDialog
{
    Q_OBJECT

public:
    explicit Tipoajustes(QWidget *parent = nullptr);
    ~Tipoajustes();

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
    Ui::Tipoajustes *ui;
    QSqlQueryModel *modelo;
};

#endif // TIPOAJUSTES_H
