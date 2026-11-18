#ifndef AJUSTES_H
#define AJUSTES_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Ajustes;
}

class Ajustes : public QDialog
{
    Q_OBJECT

public:
    explicit Ajustes(QWidget *parent = nullptr);
    ~Ajustes();

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
    Ui::Ajustes *ui;
    QSqlQueryModel *modelo;
};

#endif // AJUSTES_H
