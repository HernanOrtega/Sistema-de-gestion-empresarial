#ifndef ARQUEOS_H
#define ARQUEOS_H

#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class Arqueos;
}

class Arqueos : public QDialog
{
    Q_OBJECT

public:
    explicit Arqueos(QWidget *parent = nullptr);
    ~Arqueos();

private slots:
    void cargarCombos();
    void cargarDatos();
    void limpiarCampos();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous = QModelIndex());

    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnCerrarArqueo_clicked();  // Nuevo botón para cerrar arqueo
    void on_btnSalir_clicked();

private:
    Ui::Arqueos *ui;
    QSqlQueryModel *modelo;

    void actualizarEstadoBotones(const QString &estado);
};

#endif // ARQUEOS_H
