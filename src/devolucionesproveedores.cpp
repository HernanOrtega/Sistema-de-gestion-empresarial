#include "devolucionesproveedores.h"
#include "ui_devolucionesproveedores.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDoubleValidator>

Devolucionesproveedores::Devolucionesproveedores(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Devolucionesproveedores),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo cian moderno ------------------------------
    this->setStyleSheet("background-color: #f0fdff;"); // Fondo cian claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtTotal->setStyleSheet(lineEditStyle);
    ui->txtObservaciones->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #00838f; font-weight: bold; font-size: 14px;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelProveedor->setStyleSheet(labelStyle);
    ui->labelCompra->setStyleSheet(labelStyle);
    ui->labelFechaDevolucion->setStyleSheet(labelStyle);
    ui->labelTotal->setStyleSheet(labelStyle);
    ui->labelObservaciones->setStyleSheet(labelStyle);
    ui->labelUsuario->setStyleSheet(labelStyle);

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
    ui->cbProveedor->setStyleSheet(comboStyle);
    ui->cbCompra->setStyleSheet(comboStyle);
    ui->cbUsuario->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit
    QString dateStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->dtFechaDevolucion->setStyleSheet(dateStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaDevolucion->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaDevolucion->setDateTime(QDateTime::currentDateTime());

    // Configurar QTextEdit para observaciones
    QString textEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtObservaciones->setStyleSheet(textEditStyle);

    // Configurar validación para total (solo números decimales)
    ui->txtTotal->setValidator(new QDoubleValidator(0, 999999999.99, 2, this));

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

    ui->tablaDevoluciones->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    if (ui->tablaDevoluciones->selectionModel()) {
        connect(ui->tablaDevoluciones->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                &Devolucionesproveedores::filaSeleccionada);
    }
}

Devolucionesproveedores::~Devolucionesproveedores()
{
    delete ui;
}

void Devolucionesproveedores::cargarCombos()
{
    // Cargar proveedores activos
    ui->cbProveedor->clear();
    QSqlQuery query;
    if (!query.exec("SELECT id, razon_social FROM proveedor WHERE activo = true ORDER BY razon_social")) {
        qDebug() << "Error cargando proveedores:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los proveedores:\n%1").arg(query.lastError().text()));
        return;
    }

    while(query.next()) {
        int id = query.value(0).toInt();
        QString razonSocial = query.value(1).toString();
        ui->cbProveedor->addItem(razonSocial, id);
    }

    // Cargar compras
    ui->cbCompra->clear();
    if (!query.exec("SELECT c.id, 'Compra #' || c.id || ' - Prov: ' || COALESCE(p.razon_social, 'N/A') || ' - Total: ' || c.total || ' - Fact: ' || COALESCE(c.nro_factura, 'S/F') "
                    "FROM compra c "
                    "LEFT JOIN proveedor p ON c.proveedor_id = p.id "
                    "WHERE c.estado != 'ANULADA' "
                    "ORDER BY c.fecha_compra DESC")) {
        qDebug() << "Error cargando compras:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar las compras:\n%1").arg(query.lastError().text()));
        return;
    }

    while(query.next()) {
        int id = query.value(0).toInt();
        QString descripcion = query.value(1).toString();
        ui->cbCompra->addItem(descripcion, id);
    }

    // Cargar usuarios activos
    ui->cbUsuario->clear();
    if (!query.exec("SELECT id, username, rol FROM usuario WHERE activo = true ORDER BY username")) {
        qDebug() << "Error cargando usuarios:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los usuarios:\n%1").arg(query.lastError().text()));
        return;
    }

    while(query.next()) {
        int id = query.value(0).toInt();
        QString username = query.value(1).toString();
        QString rol = query.value(2).toString();
        QString displayText = QString("%1 (%2)").arg(username).arg(rol);
        ui->cbUsuario->addItem(displayText, id);
    }
}

void Devolucionesproveedores::cargarDatos()
{
    QString sql = "SELECT dp.id, "
                  "p.razon_social AS proveedor, "
                  "'Compra #' || c.id || ' - Fact: ' || COALESCE(c.nro_factura, 'S/F') AS compra, "
                  "TO_CHAR(dp.fecha_devolucion, 'YYYY-MM-DD HH24:MI') AS fecha_devolucion, "
                  "dp.total, "
                  "COALESCE(dp.observaciones, 'Sin observaciones') AS observaciones, "
                  "u.username || ' (' || u.rol || ')' AS usuario "
                  "FROM devolucion_a_proveedor dp "
                  "LEFT JOIN proveedor p ON dp.proveedor_id = p.id "
                  "LEFT JOIN compra c ON dp.compra_id = c.id "
                  "LEFT JOIN usuario u ON dp.usuario_id = u.id "
                  "ORDER BY dp.fecha_devolucion DESC";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Proveedor"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Compra"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Fecha Devolución"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Total"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Observaciones"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Usuario"));

    ui->tablaDevoluciones->setModel(modelo);
    ui->tablaDevoluciones->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaDevoluciones->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaDevoluciones->horizontalHeader()->setStretchLastSection(true);
    ui->tablaDevoluciones->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Ajustar el ancho de las columnas
    ui->tablaDevoluciones->setColumnWidth(0, 50);   // ID
    ui->tablaDevoluciones->setColumnWidth(1, 200);  // Proveedor
    ui->tablaDevoluciones->setColumnWidth(2, 200);  // Compra
    ui->tablaDevoluciones->setColumnWidth(3, 150);  // Fecha Devolución
    ui->tablaDevoluciones->setColumnWidth(4, 100);  // Total
    ui->tablaDevoluciones->setColumnWidth(5, 250);  // Observaciones
    ui->tablaDevoluciones->setColumnWidth(6, 150);  // Usuario

    limpiarCampos();
}

void Devolucionesproveedores::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtTotal->clear();
    ui->txtObservaciones->clear();

    if(ui->cbProveedor->count() > 0)
        ui->cbProveedor->setCurrentIndex(0);
    if(ui->cbCompra->count() > 0)
        ui->cbCompra->setCurrentIndex(0);
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->dtFechaDevolucion->setDateTime(QDateTime::currentDateTime());

    ui->cbProveedor->setFocus();
}

void Devolucionesproveedores::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Devolucionesproveedores::on_btnGuardar_clicked()
{
    int proveedorId = ui->cbProveedor->currentData().toInt();
    int compraId = ui->cbCompra->currentData().toInt();
    QDateTime fechaDevolucion = ui->dtFechaDevolucion->dateTime();
    double total = ui->txtTotal->text().toDouble();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(proveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un proveedor."));
        ui->cbProveedor->setFocus();
        return;
    }

    if(compraId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una compra."));
        ui->cbCompra->setFocus();
        return;
    }

    if(total <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El total debe ser mayor a cero."));
        ui->txtTotal->setFocus();
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        ui->cbUsuario->setFocus();
        return;
    }

    if(fechaDevolucion > QDateTime::currentDateTime()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha de devolución no puede ser futura."));
        ui->dtFechaDevolucion->setFocus();
        return;
    }

    // Verificar si ya existe una devolución para esta compra
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM devolucion_a_proveedor WHERE compra_id = ?");
    checkQuery.addBindValue(compraId);
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe una devolución para esta compra."));
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO devolucion_a_proveedor (proveedor_id, compra_id, fecha_devolucion, total, observaciones, usuario_id) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(proveedorId);
    query.addBindValue(compraId);
    query.addBindValue(fechaDevolucion);
    query.addBindValue(total);
    query.addBindValue(observaciones.isEmpty() ? QVariant() : QVariant(observaciones));
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar la devolución:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Devolución a proveedor guardada correctamente."));
}

void Devolucionesproveedores::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int proveedorId = ui->cbProveedor->currentData().toInt();
    int compraId = ui->cbCompra->currentData().toInt();
    QDateTime fechaDevolucion = ui->dtFechaDevolucion->dateTime();
    double total = ui->txtTotal->text().toDouble();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una devolución para modificar."));
        return;
    }

    if(proveedorId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un proveedor."));
        ui->cbProveedor->setFocus();
        return;
    }

    if(compraId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una compra."));
        ui->cbCompra->setFocus();
        return;
    }

    if(total <= 0) {
        QMessageBox::warning(this, tr("Atención"), tr("El total debe ser mayor a cero."));
        ui->txtTotal->setFocus();
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        ui->cbUsuario->setFocus();
        return;
    }

    if(fechaDevolucion > QDateTime::currentDateTime()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha de devolución no puede ser futura."));
        ui->dtFechaDevolucion->setFocus();
        return;
    }

    // Verificar si ya existe otra devolución para esta compra (excluyendo la actual)
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM devolucion_a_proveedor WHERE compra_id = ? AND id != ?");
    checkQuery.addBindValue(compraId);
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        QMessageBox::warning(this, tr("Atención"), tr("Ya existe otra devolución para esta compra."));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE devolucion_a_proveedor SET proveedor_id=?, compra_id=?, fecha_devolucion=?, total=?, observaciones=?, usuario_id=? WHERE id=?");
    query.addBindValue(proveedorId);
    query.addBindValue(compraId);
    query.addBindValue(fechaDevolucion);
    query.addBindValue(total);
    query.addBindValue(observaciones.isEmpty() ? QVariant() : QVariant(observaciones));
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar la devolución:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Devolución a proveedor modificada correctamente."));
}

void Devolucionesproveedores::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione una devolución para eliminar."));
        return;
    }

    // Verificar si hay detalles de devolución que dependen de esta devolución
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM devolucion_detalle WHERE devolucion_id = ?");
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        int count = checkQuery.value(0).toInt();
        if(count > 0) {
            QMessageBox::warning(this, tr("No se puede eliminar"),
                                 tr("No se puede eliminar esta devolución porque tiene %1 detalle(s) asociado(s).").arg(count));
            return;
        }
    }

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar esta devolución?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM devolucion_a_proveedor WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar la devolución:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Devolución a proveedor eliminada correctamente."));
}

void Devolucionesproveedores::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los datos directamente de la base de datos para una búsqueda precisa
    int idDevolucion = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT proveedor_id, compra_id, fecha_devolucion, total, observaciones, usuario_id "
                  "FROM devolucion_a_proveedor WHERE id = ?");
    query.addBindValue(idDevolucion);

    if(query.exec() && query.next()) {
        int proveedorId = query.value(0).toInt();
        int compraId = query.value(1).toInt();
        QDateTime fechaDevolucion = query.value(2).toDateTime();
        double total = query.value(3).toDouble();
        QString observaciones = query.value(4).toString();
        int usuarioId = query.value(5).toInt();

        // Buscar por ID en lugar de por texto
        int indexProveedor = ui->cbProveedor->findData(proveedorId);
        if(indexProveedor >= 0) {
            ui->cbProveedor->setCurrentIndex(indexProveedor);
        }

        int indexCompra = ui->cbCompra->findData(compraId);
        if(indexCompra >= 0) {
            ui->cbCompra->setCurrentIndex(indexCompra);
        }

        int indexUsuario = ui->cbUsuario->findData(usuarioId);
        if(indexUsuario >= 0) {
            ui->cbUsuario->setCurrentIndex(indexUsuario);
        }

        if(fechaDevolucion.isValid()) {
            ui->dtFechaDevolucion->setDateTime(fechaDevolucion);
        }

        ui->txtTotal->setText(QString::number(total, 'f', 2));
        ui->txtObservaciones->setPlainText(observaciones);
    } else {
        qDebug() << "Error al cargar datos de la devolución:" << query.lastError().text();
    }
}

void Devolucionesproveedores::on_btnSalir_clicked()
{
    close();
}
