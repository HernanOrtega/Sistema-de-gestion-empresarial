#ifndef MARCAS_H
#define MARCAS_H

#include <QDialog>
#include <QSqlTableModel>
#include <QStyledItemDelegate>

namespace Ui {
class Marcas;
}

// Delegate para mostrar Sí/No en vez de true/false
class BooleanDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QString displayText(const QVariant &value, const QLocale &locale) const override {
        Q_UNUSED(locale)
        return value.toBool() ? "Sí" : "No";
    }
};

class Marcas : public QDialog
{
    Q_OBJECT

public:
    explicit Marcas(QWidget *parent = nullptr);
    ~Marcas();

private slots:
    void on_btnNuevo_clicked();
    void on_btnGuardar_clicked();
    void on_btnModificar_clicked();
    void on_btnEliminar_clicked();
    void on_btnCerrar_clicked();
    void filaSeleccionada(const QModelIndex &current, const QModelIndex &previous);

private:
    Ui::Marcas *ui;
    QSqlTableModel *modelo;

    void cargarDatos();
    void limpiarCampos();
};

#endif // MARCAS_H

