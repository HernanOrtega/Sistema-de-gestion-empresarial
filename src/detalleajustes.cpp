#include "detalleajustes.h"
#include "ui_detalleajustes.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QIntValidator>

Detalleajustes::Detalleajustes(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Detalleajustes),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo cian moderno ------------------------------
    this->setStyleSheet("background-color: #f0fdff;"); // Fondo cian claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtCantidadAnterior->setStyleSheet(lineEditStyle);
    ui->txtCantidadNueva->setStyleSheet(lineEditStyle);
    ui->txtDiferencia->setStyleSheet(lineEditStyle);
    ui->txtMotivo->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #00838f; font-weight: bold; font-size: 14px;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelAjuste->setStyleSheet(labelStyle);
    ui->labelProducto->setStyleSheet(labelStyle);
    ui->labelCantidadAnterior->setStyleSheet(labelStyle);
    ui->labelCantidadNueva->setStyleSheet(labelStyle);
    ui->labelDiferencia->setStyleSheet(labelStyle);
    ui->labelMotivo->setStyleSheet(labelStyle);

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

    // Estilo para QComboBox
    QString comboStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->cbAjuste->setStyleSheet(comboStyle);
    ui->cbProducto->setStyleSheet(comboStyle);

    ui->txtID->setEnabled(false);
    ui->txtDiferencia->setEnabled(false); // La diferencia se calcula automáticamente

    // Configurar QTextEdit para motivo
    QString textEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtMotivo->setStyleSheet(textEditStyle);

    // Configurar validación para cantidades (solo números enteros)
    ui->txtCantidadAnterior->setValidator(new QIntValidator(0, 999999, this));
    ui->txtCantidadNueva->setValidator(new QIntValidator(0, 999999, this));

    // Conectar señales para calcular diferencia automáticamente
    connect(ui->txtCantidadAnterior, &QLineEdit::textChanged, this, &Detalleajustes::calcularDiferencia);
    connect(ui->txtCantidadNueva, &QLineEdit::textChanged, this, &Detalleajustes::calcularDiferencia);

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

    ui->tablaDetallesAjuste->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    if (ui->tablaDetallesAjuste->selectionModel()) {
        connect(ui->tablaDetallesAjuste->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                &Detalleajustes::filaSeleccionada);
    }
}

Detalleajustes::~Detalleajustes()
{
    delete ui;
}

void Detalleajustes::cargarCombos()
{
    // Cargar ajustes
    ui->cbAjuste->clear();
    QSqlQuery query;
    if (!query.exec("SELECT a.id, 'Ajuste #' || a.id || ' - ' || at.nombre || ' - ' || TO_CHAR(a.fecha_ajuste, 'YYYY-MM-DD') "
                    "FROM ajustes a "
                    "LEFT JOIN ajustes_tipo at ON a.ajuste_tipo_id = at.id "
                    "ORDER BY a.fecha_ajuste DESC")) {
        qDebug() << "Error cargando ajustes:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los ajustes:\n%1").arg(query.lastError().text()));
        return;
    }

    while(query.next()) {
        int id = query.value(0).toInt();
        QString descripcion = query.value(1).toString();
        ui->cbAjuste->addItem(descripcion, id);
    }

    // Cargar productos activos
    ui->cbProducto->clear();
    if (!query.exec("SELECT id, nombre, codigo FROM producto WHERE activo = true ORDER BY nombre")) {
        qDebug() << "Error cargando productos:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los productos:\n%1").arg(query.lastError().text()));
        return;
    }

    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        QString codigo = query.value(2).toString();
        QString displayText = QString("%1 (%2)").arg(nombre).arg(codigo);
        ui->cbProducto->addItem(displayText, id);
    }
}

void Detalleajustes::cargarDatos()
{
    QString sql = "SELECT ad.id, "
                  "'Ajuste #' || a.id || ' - ' || at.nombre AS ajuste, "
                  "p.nombre || ' (' || p.codigo || ')' AS producto, "
                  "ad.cantidad_anterior, "
                  "ad.cantidad_nueva, "
                  "ad.diferencia, "
                  "COALESCE(ad.motivo, 'Sin motivo') AS motivo "
                  "FROM ajuste_detalle ad "
                  "LEFT JOIN ajustes a ON ad.ajuste_id = a.id "
                  "LEFT JOIN ajustes_tipo at ON a.ajuste_tipo_id = at.id "
                  "LEFT JOIN producto p ON ad.producto_id = p.id "
                  "ORDER BY a.fecha_ajuste DESC, p.nombre";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Ajuste"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Producto"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Cant. Anterior"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Cant. Nueva"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Diferencia"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Motivo"));

    ui->tablaDetallesAjuste->setModel(modelo);
    ui->tablaDetallesAjuste->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDetallesAjuste->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDetallesAjuste->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDetallesAjuste->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Ajustar el ancho de las columnas
    ui->tablaDetallesAjuste->setColumnWidth(0, 50);   // ID
    ui->tablaDetallesAjuste->setColumnWidth(1, 200);  // Ajuste
    ui->tablaDetallesAjuste->setColumnWidth(2, 200);  // Producto
    ui->tablaDetallesAjuste->setColumnWidth(3, 100);  // Cant. Anterior
    ui->tablaDetallesAjuste->setColumnWidth(4, 100);  // Cant. Nueva
    ui->tablaDetallesAjuste->setColumnWidth(5, 80);   // Diferencia
    ui->tablaDetallesAjuste->setColumnWidth(6, 200);  // Motivo

    limpiarCampos();
}

void Detalleajustes::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtCantidadAnterior->clear();
    ui->txtCantidadNueva->clear();
    ui->txtDiferencia->clear();
    ui->txtMotivo->clear();

    if(ui->cbAjuste->count() > 0)
        ui->cbAjuste->setCurrentIndex(0);
    if(ui->cbProducto->count() > 0)
        ui->cbProducto->setCurrentIndex(0);

    ui->cbAjuste->setFocus();
}

void Detalleajustes::calcularDiferencia()
{
    int cantidadAnterior = ui->txtCantidadAnterior->text().toInt();
    int cantidadNueva = ui->txtCantidadNueva->text().toInt();
    int diferencia = cantidadNueva - cantidadAnterior;

    ui->txtDiferencia->setText(QString::number(diferencia));

    // Cambiar color según si es positivo o negativo
    if (diferencia > 0) {
        ui->txtDiferencia->setStyleSheet("background-color: #e8f5e8; color: #2e7d32; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;");
    } else if (diferencia < 0) {
        ui->txtDiferencia->setStyleSheet("background-color: #ffebee; color: #c62828; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;");
    } else {
        ui->txtDiferencia->setStyleSheet("background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;");
    }
}

void Detalleajustes::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Detalleajustes::on_btnGuardar_clicked()
{
    int ajusteId = ui->cbAjuste->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidadAnterior = ui->txtCantidadAnterior->text().toInt();
    int cantidadNueva = ui->txtCantidadNueva->text().toInt();
    int diferencia = ui->txtDiferencia->text().toInt();
    QString motivo = ui->txtMotivo->toPlainText().trimmed();

    if(ajusteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un ajuste."));
        ui->cbAjuste->setFocus();
        return;
    }

    if(productoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un producto."));
        ui->cbProducto->setFocus();
        return;
    }

    if(cantidadAnterior < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad anterior no puede ser negativa."));
        ui->txtCantidadAnterior->setFocus();
        return;
    }

    if(cantidadNueva < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad nueva no puede ser negativa."));
        ui->txtCantidadNueva->setFocus();
        return;
    }

    // Verificar si ya existe un detalle para este ajuste y producto
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM ajuste_detalle WHERE ajuste_id = ? AND producto_id = ?");
    checkQuery.addBindValue(ajusteId);
    checkQuery.addBindValue(productoId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe un detalle para este ajuste y producto."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO ajuste_detalle (ajuste_id, producto_id, cantidad_anterior, cantidad_nueva, diferencia, motivo) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(ajusteId);
    query.addBindValue(productoId);
    query.addBindValue(cantidadAnterior);
    query.addBindValue(cantidadNueva);
    query.addBindValue(diferencia);
    query.addBindValue(motivo.isEmpty() ? QVariant() : QVariant(motivo));

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el detalle del ajuste:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Detalle de ajuste guardado correctamente."));
}

void Detalleajustes::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int ajusteId = ui->cbAjuste->currentData().toInt();
    int productoId = ui->cbProducto->currentData().toInt();
    int cantidadAnterior = ui->txtCantidadAnterior->text().toInt();
    int cantidadNueva = ui->txtCantidadNueva->text().toInt();
    int diferencia = ui->txtDiferencia->text().toInt();
    QString motivo = ui->txtMotivo->toPlainText().trimmed();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle de ajuste para modificar."));
        return;
    }

    if(ajusteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un ajuste."));
        ui->cbAjuste->setFocus();
        return;
    }

    if(productoId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un producto."));
        ui->cbProducto->setFocus();
        return;
    }

    if(cantidadAnterior < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad anterior no puede ser negativa."));
        ui->txtCantidadAnterior->setFocus();
        return;
    }

    if(cantidadNueva < 0) {
        QMessageBox::warning(this, tr("Atención"), tr("La cantidad nueva no puede ser negativa."));
        ui->txtCantidadNueva->setFocus();
        return;
    }

    // Verificar si ya existe otro detalle para este ajuste y producto (excluyendo el actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM ajuste_detalle WHERE ajuste_id = ? AND producto_id = ? AND id != ?");
    checkQuery.addBindValue(ajusteId);
    checkQuery.addBindValue(productoId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otro detalle para este ajuste y producto."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE ajuste_detalle SET ajuste_id=?, producto_id=?, cantidad_anterior=?, cantidad_nueva=?, diferencia=?, motivo=? WHERE id=?");
    query.addBindValue(ajusteId);
    query.addBindValue(productoId);
    query.addBindValue(cantidadAnterior);
    query.addBindValue(cantidadNueva);
    query.addBindValue(diferencia);
    query.addBindValue(motivo.isEmpty() ? QVariant() : QVariant(motivo));
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el detalle del ajuste:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Detalle de ajuste modificado correctamente."));
}

void Detalleajustes::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un detalle de ajuste para eliminar."));
        return;
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este detalle de ajuste?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM ajuste_detalle WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el detalle del ajuste:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Detalle de ajuste eliminado correctamente."));
}

void Detalleajustes::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los datos directamente de la base de datos para una búsqueda precisa
    int idDetalle = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT ajuste_id, producto_id, cantidad_anterior, cantidad_nueva, diferencia, motivo "
                  "FROM ajuste_detalle WHERE id = ?");
    query.addBindValue(idDetalle);

    if(query.exec() && query.next()) {
        int ajusteId = query.value(0).toInt();
        int productoId = query.value(1).toInt();
        int cantidadAnterior = query.value(2).toInt();
        int cantidadNueva = query.value(3).toInt();
        int diferencia = query.value(4).toInt();
        QString motivo = query.value(5).toString();

        // Buscar por ID en lugar de por texto
        int indexAjuste = ui->cbAjuste->findData(ajusteId);
        if(indexAjuste >= 0) {
            ui->cbAjuste->setCurrentIndex(indexAjuste);
        }

        int indexProducto = ui->cbProducto->findData(productoId);
        if(indexProducto >= 0) {
            ui->cbProducto->setCurrentIndex(indexProducto);
        }

        ui->txtCantidadAnterior->setText(QString::number(cantidadAnterior));
        ui->txtCantidadNueva->setText(QString::number(cantidadNueva));
        ui->txtDiferencia->setText(QString::number(diferencia));
        ui->txtMotivo->setPlainText(motivo);

        // Actualizar el color de la diferencia
        calcularDiferencia();
    } else {
        qDebug() << "Error al cargar datos del detalle de ajuste:" << query.lastError().text();
    }
}

void Detalleajustes::on_btnSalir_clicked()
{
    close();
}
