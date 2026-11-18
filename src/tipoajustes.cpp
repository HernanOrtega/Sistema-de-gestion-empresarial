#include "tipoajustes.h"
#include "ui_tipoajustes.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Tipoajustes::Tipoajustes(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Tipoajustes),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo cian moderno ------------------------------
    this->setStyleSheet("background-color: #f0fdff;"); // Fondo cian claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtNombre->setStyleSheet(lineEditStyle);
    ui->txtDescripcion->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #00838f; font-weight: bold; font-size: 14px;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelNombre->setStyleSheet(labelStyle);
    ui->labelDescripcion->setStyleSheet(labelStyle);
    ui->labelAfectaStock->setStyleSheet(labelStyle);

    // Estilo para botones en tono cian moderno
    QString btnStyle = R"(
        QPushButton {
            background-color: #00bcd4;
            color: #ffffff;
            font-weight: bold;
            border-radius: 8px;
            padding: 8px 16px;
            border: none;
            font-size: 14px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #00acc1;
        }
        QPushButton:pressed {
            background-color: #0097a7;
            background-color: #00838f;
        }
        QPushButton:disabled {
            background-color: #b2ebf2;
            color: #80deea;
        }
    )";

    ui->btnNuevo->setStyleSheet(btnStyle);
    ui->btnGuardar->setStyleSheet(btnStyle);
    ui->btnModificar->setStyleSheet(btnStyle);
    ui->btnEliminar->setStyleSheet(btnStyle);
    ui->btnSalir->setStyleSheet(btnStyle);

    // Estilo para QCheckBox
    QString checkBoxStyle = R"(
        QCheckBox {
            color: #00838f;
            font-weight: bold;
            font-size: 14px;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #00bcd4;
            border-radius: 4px;
            background-color: white;
        }
        QCheckBox::indicator:checked {
            background-color: #00bcd4;
            border: 2px solid #00838f;
        }
        QCheckBox::indicator:hover {
            border: 2px solid #0097a7;
        }
    )";
    ui->chkAfectaStock->setStyleSheet(checkBoxStyle);

    ui->txtID->setEnabled(false);

    // Configurar QTextEdit para descripción
    QString textEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtDescripcion->setStyleSheet(textEditStyle);

    // ------------------------------ Configuración tabla en tonos cian ------------------------------
    QString tablaStyle = R"(
        QTableView {
            background-color: #e0f7fa;
            alternate-background-color: #b2ebf2;
            gridline-color: #00bcd4;
            selection-background-color: #00bcd4;
            selection-color: white;
            border: 2px solid #00bcd4;
            border-radius: 8px;
            font-size: 13px;
        }
        QHeaderView::section {
            background-color: #00838f;
            color: white;
            font-weight: bold;
            border: 1px solid #006064;
            padding: 8px;
            font-size: 14px;
        }
        QTableView::item {
            padding: 6px;
            border-bottom: 1px solid #80deea;
            color: #006064;
        }
        QTableView::item:selected {
            background-color: #00bcd4;
            color: white;
            font-weight: bold;
        }
        QTableView::item:hover {
            background-color: #b2ebf2;
        }
    )";

    ui->tablaTiposAjuste->setStyleSheet(tablaStyle);

    // Cargar datos
    cargarDatos();

    // Conectar la señal después de setModel
    if (ui->tablaTiposAjuste->selectionModel()) {
        connect(ui->tablaTiposAjuste->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                &Tipoajustes::filaSeleccionada);
    }
}

Tipoajustes::~Tipoajustes()
{
    delete ui;
}

void Tipoajustes::cargarDatos()
{
    QString sql = "SELECT id, nombre, descripcion, afecta_stock "
                  "FROM ajustes_tipo "
                  "ORDER BY nombre";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Nombre"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Descripción"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Afecta Stock"));

    ui->tablaTiposAjuste->setModel(modelo);
    ui->tablaTiposAjuste->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaTiposAjuste->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaTiposAjuste->horizontalHeader()->setStretchLastSection(true);
    ui->tablaTiposAjuste->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Ajustar el ancho de las columnas
    ui->tablaTiposAjuste->setColumnWidth(0, 50);   // ID
    ui->tablaTiposAjuste->setColumnWidth(1, 200);  // Nombre
    ui->tablaTiposAjuste->setColumnWidth(2, 300);  // Descripción
    ui->tablaTiposAjuste->setColumnWidth(3, 100);  // Afecta Stock

    limpiarCampos();
}

void Tipoajustes::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtNombre->clear();
    ui->txtDescripcion->clear();
    ui->chkAfectaStock->setChecked(true); // Por defecto afecta stock

    ui->txtNombre->setFocus();
}

void Tipoajustes::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Tipoajustes::on_btnGuardar_clicked()
{
    QString nombre = ui->txtNombre->text().trimmed();
    QString descripcion = ui->txtDescripcion->toPlainText().trimmed();
    bool afectaStock = ui->chkAfectaStock->isChecked();

    if(nombre.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("El nombre es obligatorio."));
        ui->txtNombre->setFocus();
        return;
    }

    if(nombre.length() > 100) {
        QMessageBox::warning(this, tr("Atención"), tr("El nombre no puede tener más de 100 caracteres."));
        ui->txtNombre->setFocus();
        return;
    }

    // Verificar si ya existe un tipo de ajuste con el mismo nombre
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM ajustes_tipo WHERE nombre = ?");
    checkQuery.addBindValue(nombre);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe un tipo de ajuste con este nombre."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO ajustes_tipo (nombre, descripcion, afecta_stock) "
                  "VALUES (?, ?, ?)");
    query.addBindValue(nombre);
    query.addBindValue(descripcion.isEmpty() ? QVariant() : QVariant(descripcion));
    query.addBindValue(afectaStock);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el tipo de ajuste:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Tipo de ajuste guardado correctamente."));
}

void Tipoajustes::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    QString nombre = ui->txtNombre->text().trimmed();
    QString descripcion = ui->txtDescripcion->toPlainText().trimmed();
    bool afectaStock = ui->chkAfectaStock->isChecked();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un tipo de ajuste para modificar."));
        return;
    }

    if(nombre.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("El nombre es obligatorio."));
        ui->txtNombre->setFocus();
        return;
    }

    if(nombre.length() > 100) {
        QMessageBox::warning(this, tr("Atención"), tr("El nombre no puede tener más de 100 caracteres."));
        ui->txtNombre->setFocus();
        return;
    }

    // Verificar si ya existe otro tipo de ajuste con el mismo nombre (excluyendo el actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM ajustes_tipo WHERE nombre = ? AND id != ?");
    checkQuery.addBindValue(nombre);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otro tipo de ajuste con este nombre."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE ajustes_tipo SET nombre=?, descripcion=?, afecta_stock=? WHERE id=?");
    query.addBindValue(nombre);
    query.addBindValue(descripcion.isEmpty() ? QVariant() : QVariant(descripcion));
    query.addBindValue(afectaStock);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el tipo de ajuste:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Tipo de ajuste modificado correctamente."));
}

void Tipoajustes::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un tipo de ajuste para eliminar."));
        return;
    }

    // COMENTADO: Verificación de tabla ajustes_stock que no existe
    /*
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM ajustes_stock WHERE tipo_ajuste_id = ?");
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        int count = checkQuery.value(0).toInt();
        if(count > 0) {
            QMessageBox::warning(this, tr("No se puede eliminar"),
                                tr("No se puede eliminar este tipo de ajuste porque está siendo utilizado por %1 ajuste(s) de stock.").arg(count));
            return;
        }
    }
    */

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este tipo de ajuste?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM ajustes_tipo WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el tipo de ajuste:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Tipo de ajuste eliminado correctamente."));
}

void Tipoajustes::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los datos directamente de la base de datos para una búsqueda precisa
    int idTipo = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT nombre, descripcion, afecta_stock "
                  "FROM ajustes_tipo WHERE id = ?");
    query.addBindValue(idTipo);

    if(query.exec() && query.next()) {
        QString nombre = query.value(0).toString();
        QString descripcion = query.value(1).toString();
        bool afectaStock = query.value(2).toBool();

        ui->txtNombre->setText(nombre);
        ui->txtDescripcion->setPlainText(descripcion);
        ui->chkAfectaStock->setChecked(afectaStock);
    } else {
        qDebug() << "Error al cargar datos del tipo de ajuste:" << query.lastError().text();
    }
}

void Tipoajustes::on_btnSalir_clicked()
{
    close();
}
