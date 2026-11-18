#include "auditoria.h"
#include "ui_auditoria.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>
#include <QDate>
#include <QHeaderView>
#include <QTimer>

Auditoria::Auditoria(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Auditoria),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    qDebug() << "Inicializando formulario de Auditoria...";

    // ------------------------------ Estilo oscuro moderno ------------------------------
    this->setStyleSheet("background-color: #2b2b2b; color: #e0e0e0;"); // Fondo gris oscuro

    QString lineEditStyle = "background-color: #3c3c3c; color: #ffffff; border: 1px solid #555555; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtTablaAfectada->setStyleSheet(lineEditStyle);
    ui->txtRegistroID->setStyleSheet(lineEditStyle);
    ui->txtAccion->setStyleSheet(lineEditStyle);
    ui->txtValorAnterior->setStyleSheet(lineEditStyle);
    ui->txtValorNuevo->setStyleSheet(lineEditStyle);
    ui->txtUsuario->setStyleSheet(lineEditStyle);
    ui->txtIP->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #bb86fc; font-weight: bold; font-size: 14px;"; // Color morado claro
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelTablaAfectada->setStyleSheet(labelStyle);
    ui->labelRegistroID->setStyleSheet(labelStyle);
    ui->labelAccion->setStyleSheet(labelStyle);
    ui->labelValorAnterior->setStyleSheet(labelStyle);
    ui->labelValorNuevo->setStyleSheet(labelStyle);
    ui->labelUsuario->setStyleSheet(labelStyle);
    ui->labelFechaAccion->setStyleSheet(labelStyle);
    ui->labelIP->setStyleSheet(labelStyle);
    ui->labelFiltroTabla->setStyleSheet(labelStyle);
    ui->labelFiltroAccion->setStyleSheet(labelStyle);
    ui->labelFiltroFechaDesde->setStyleSheet(labelStyle);
    ui->labelFiltroFechaHasta->setStyleSheet(labelStyle);

    // Estilo para botones en tono morado oscuro
    QString btnStyle = R"(
        QPushButton {
            background-color: #6200ee;
            color: #ffffff;
            font-weight: bold;
            border-radius: 8px;
            padding: 8px 16px;
            border: none;
            font-size: 14px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #7c4dff;
        }
        QPushButton:pressed {
            background-color: #3700b3;
        }
        QPushButton:disabled {
            background-color: #424242;
            color: #666666;
        }
    )";

    ui->btnLimpiarFiltros->setStyleSheet(btnStyle);
    ui->btnAplicarFiltros->setStyleSheet(btnStyle);
    ui->btnSalir->setStyleSheet(btnStyle);

    // Estilo para QComboBox
    QString comboStyle = "background-color: #3c3c3c; color: #ffffff; border: 1px solid #555555; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->cbFiltroTabla->setStyleSheet(comboStyle);
    ui->cbFiltroAccion->setStyleSheet(comboStyle);

    // Estilo para QDateEdit y QDateTimeEdit
    QString dateStyle = "background-color: #3c3c3c; color: #ffffff; border: 1px solid #555555; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->deFechaDesde->setStyleSheet(dateStyle);
    ui->deFechaHasta->setStyleSheet(dateStyle);
    ui->deFechaAccion->setStyleSheet(dateStyle);

    // Estilo para QTextEdit
    QString textEditStyle = "background-color: #3c3c3c; color: #ffffff; border: 1px solid #555555; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtValorAnterior->setStyleSheet(textEditStyle);
    ui->txtValorNuevo->setStyleSheet(textEditStyle);

    ui->txtID->setEnabled(false);
    ui->txtTablaAfectada->setEnabled(false);
    ui->txtRegistroID->setEnabled(false);
    ui->txtAccion->setEnabled(false);
    ui->txtValorAnterior->setEnabled(false);
    ui->txtValorNuevo->setEnabled(false);
    ui->txtUsuario->setEnabled(false);
    ui->txtIP->setEnabled(false);
    ui->deFechaAccion->setEnabled(false);

    // Configurar fechas para filtros
    ui->deFechaDesde->setDisplayFormat("yyyy-MM-dd");
    ui->deFechaHasta->setDisplayFormat("yyyy-MM-dd");
    ui->deFechaAccion->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    ui->deFechaDesde->setDate(QDate::currentDate().addDays(-30)); // Últimos 30 días por defecto
    ui->deFechaHasta->setDate(QDate::currentDate());

    // Configurar combos de filtros
    ui->cbFiltroTabla->clear();
    ui->cbFiltroTabla->addItem(tr("Todas las tablas"), "");

    ui->cbFiltroAccion->clear();
    ui->cbFiltroAccion->addItem(tr("Todas las acciones"), "");
    ui->cbFiltroAccion->addItem("INSERT", "INSERT");
    ui->cbFiltroAccion->addItem("UPDATE", "UPDATE");
    ui->cbFiltroAccion->addItem("DELETE", "DELETE");

    // ------------------------------ Configuración tabla en tonos oscuros ------------------------------
    QString tablaStyle = R"(
        QTableView {
            background-color: #1e1e1e;
            alternate-background-color: #2d2d2d;
            gridline-color: #424242;
            selection-background-color: #6200ee;
            selection-color: white;
            border: 2px solid #424242;
            border-radius: 8px;
            font-size: 13px;
            color: #e0e0e0;
        }
        QHeaderView::section {
            background-color: #3700b3;
            color: white;
            font-weight: bold;
            border: 1px solid #1a1a1a;
            padding: 8px;
            font-size: 14px;
        }
        QTableView::item {
            padding: 6px;
            border-bottom: 1px solid #424242;
            color: #e0e0e0;
        }
        QTableView::item:selected {
            background-color: #6200ee;
            color: white;
            font-weight: bold;
        }
        QTableView::item:hover {
            background-color: #424242;
        }
    )";

    ui->tablaAuditoria->setStyleSheet(tablaStyle);

    // Cargar datos
    cargarDatos();

    // Conectar señales DESPUÉS de cargar datos y configurar el modelo
    if (ui->tablaAuditoria->selectionModel()) {
        connect(ui->tablaAuditoria->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                &Auditoria::filaSeleccionada);
        qDebug() << "Señal de selección conectada correctamente";
    } else {
        qDebug() << "Advertencia: selectionModel no disponible inmediatamente";
        // Intentar reconectar después de un breve delay
        QTimer::singleShot(100, this, [this]() {
            if (ui->tablaAuditoria->selectionModel()) {
                connect(ui->tablaAuditoria->selectionModel(),
                        &QItemSelectionModel::currentRowChanged,
                        this,
                        &Auditoria::filaSeleccionada);
                qDebug() << "Señal de selección conectada con delay";
            }
        });
    }

    // Conectar botones usando la forma tradicional de Qt
    connect(ui->btnLimpiarFiltros, &QPushButton::clicked, this, &Auditoria::on_btnLimpiarFiltros_clicked);
    connect(ui->btnAplicarFiltros, &QPushButton::clicked, this, &Auditoria::on_btnAplicarFiltros_clicked);
    connect(ui->btnSalir, &QPushButton::clicked, this, &Auditoria::on_btnSalir_clicked);

    qDebug() << "Formulario de Auditoria inicializado correctamente";
}

Auditoria::~Auditoria()
{
    qDebug() << "Destruyendo formulario de Auditoria...";
    delete ui;
}

void Auditoria::cargarDatos()
{
    qDebug() << "Cargando datos de auditoria...";

    // Verificar si la base de datos está abierta
    if (!QSqlDatabase::database().isOpen()) {
        qDebug() << "Error: No hay conexión a la base de datos";
        QMessageBox::critical(this, tr("Error de conexión"),
                              tr("No hay conexión a la base de datos."));
        return;
    }

    QString sql = "SELECT a.id, "
                  "a.tabla_afectada, "
                  "a.registro_id, "
                  "a.accion, "
                  "COALESCE(a.valor_anterior, 'Sin valor anterior') AS valor_anterior, "
                  "COALESCE(a.valor_nuevo, 'Sin valor nuevo') AS valor_nuevo, "
                  "COALESCE(u.username || ' (' || u.rol || ')', 'Usuario ' || a.usuario_id::text) AS usuario, "
                  "a.fecha_accion, "
                  "COALESCE(a.ip_address, 'N/A') AS ip_address "
                  "FROM auditoria a "
                  "LEFT JOIN usuario u ON a.usuario_id = u.id "
                  "ORDER BY a.fecha_accion DESC";

    qDebug() << "Ejecutando consulta SQL:" << sql;

    // Limpiar el modelo antes de establecer nueva consulta
    modelo->clear();
    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();

        // Intentar con consulta más simple si falla
        QString sqlSimple = "SELECT id, tabla_afectada, registro_id, accion, "
                            "COALESCE(valor_anterior, 'N/A') as valor_anterior, "
                            "COALESCE(valor_nuevo, 'N/A') as valor_nuevo, "
                            "COALESCE(usuario_id::text, '1') as usuario, "
                            "fecha_accion, "
                            "COALESCE(ip_address, 'N/A') as ip_address "
                            "FROM auditoria "
                            "ORDER BY fecha_accion DESC";

        qDebug() << "Intentando consulta simple...";
        modelo->setQuery(sqlSimple);

        if(modelo->lastError().isValid()) {
            QMessageBox::critical(this, tr("Error en consulta"),
                                  tr("No se pudieron cargar los datos de auditoría:\n%1").arg(modelo->lastError().text()));
            return;
        }
    }

    qDebug() << "Consulta ejecutada correctamente. Filas encontradas:" << modelo->rowCount();

    // Configurar headers
    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Tabla"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Registro ID"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Acción"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Valor Anterior"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Valor Nuevo"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Usuario"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Fecha Acción"));
    modelo->setHeaderData(8, Qt::Horizontal, tr("IP"));

    // Configurar tabla
    ui->tablaAuditoria->setModel(modelo);
    ui->tablaAuditoria->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaAuditoria->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaAuditoria->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Ajustar el ancho de las columnas
    ui->tablaAuditoria->setColumnWidth(0, 50);   // ID
    ui->tablaAuditoria->setColumnWidth(1, 120);  // Tabla
    ui->tablaAuditoria->setColumnWidth(2, 80);   // Registro ID
    ui->tablaAuditoria->setColumnWidth(3, 80);   // Acción
    ui->tablaAuditoria->setColumnWidth(4, 200);  // Valor Anterior
    ui->tablaAuditoria->setColumnWidth(5, 200);  // Valor Nuevo
    ui->tablaAuditoria->setColumnWidth(6, 150);  // Usuario
    ui->tablaAuditoria->setColumnWidth(7, 150);  // Fecha Acción
    ui->tablaAuditoria->setColumnWidth(8, 120);  // IP

    // Hacer que la última columna se expanda
    ui->tablaAuditoria->horizontalHeader()->setStretchLastSection(true);

    // Cargar combos de filtros con datos reales
    QSqlQuery query;

    // Cargar tablas únicas
    if (query.exec("SELECT DISTINCT tabla_afectada FROM auditoria WHERE tabla_afectada IS NOT NULL ORDER BY tabla_afectada")) {
        while(query.next()) {
            QString tabla = query.value(0).toString();
            if (!tabla.isEmpty()) {
                ui->cbFiltroTabla->addItem(tabla, tabla);
            }
        }
        qDebug() << "Tablas únicas cargadas en combo box";
    } else {
        qDebug() << "Error cargando tablas únicas:" << query.lastError().text();
    }

    // Seleccionar primera fila si hay datos
    if (modelo->rowCount() > 0) {
        QModelIndex firstIndex = modelo->index(0, 0);
        ui->tablaAuditoria->setCurrentIndex(firstIndex);
        filaSeleccionada(firstIndex, QModelIndex());
        qDebug() << "Primera fila seleccionada automáticamente";
    } else {
        qDebug() << "No hay datos en la tabla de auditoría";
        QMessageBox::information(this, tr("Información"),
                                 tr("La tabla de auditoría está vacía.\n"
                                    "Realice algunas operaciones en el sistema (agregar, editar o eliminar registros) para ver los registros de auditoría."));
    }

    qDebug() << "Datos cargados exitosamente";
}

void Auditoria::limpiarFiltros()
{
    qDebug() << "Limpiando filtros...";

    ui->cbFiltroTabla->setCurrentIndex(0);
    ui->cbFiltroAccion->setCurrentIndex(0);
    ui->deFechaDesde->setDate(QDate::currentDate().addDays(-30));
    ui->deFechaHasta->setDate(QDate::currentDate());

    cargarDatos();
}

void Auditoria::aplicarFiltros()
{
    qDebug() << "Aplicando filtros...";

    QString filtroTabla = ui->cbFiltroTabla->currentData().toString();
    QString filtroAccion = ui->cbFiltroAccion->currentData().toString();
    QDate fechaDesde = ui->deFechaDesde->date();
    QDate fechaHasta = ui->deFechaHasta->date();

    QString sql = "SELECT a.id, "
                  "a.tabla_afectada, "
                  "a.registro_id, "
                  "a.accion, "
                  "COALESCE(a.valor_anterior, 'Sin valor anterior') AS valor_anterior, "
                  "COALESCE(a.valor_nuevo, 'Sin valor nuevo') AS valor_nuevo, "
                  "COALESCE(u.username || ' (' || u.rol || ')', 'Sistema') AS usuario, "
                  "a.fecha_accion, "
                  "COALESCE(a.ip_address, 'N/A') AS ip_address "
                  "FROM auditoria a "
                  "LEFT JOIN usuario u ON a.usuario_id = u.id "
                  "WHERE 1=1";

    QList<QString> condiciones;
    QList<QVariant> valores;

    if (!filtroTabla.isEmpty()) {
        condiciones << "a.tabla_afectada = ?";
        valores << filtroTabla;
    }

    if (!filtroAccion.isEmpty()) {
        condiciones << "a.accion = ?";
        valores << filtroAccion;
    }

    if (fechaDesde.isValid()) {
        condiciones << "DATE(a.fecha_accion) >= ?";
        valores << fechaDesde.toString("yyyy-MM-dd");
    }

    if (fechaHasta.isValid()) {
        condiciones << "DATE(a.fecha_accion) <= ?";
        valores << fechaHasta.toString("yyyy-MM-dd");
    }

    if (!condiciones.isEmpty()) {
        sql += " AND " + condiciones.join(" AND ");
    }

    sql += " ORDER BY a.fecha_accion DESC";

    qDebug() << "Ejecutando consulta filtrada:" << sql;
    qDebug() << "Parámetros:" << valores;

    QSqlQuery query;
    query.prepare(sql);

    // Asignar parámetros
    for (int i = 0; i < valores.size(); ++i) {
        query.addBindValue(valores[i]);
    }

    if (!query.exec()) {
        qDebug() << "Error en consulta filtrada:" << query.lastError().text();
        QMessageBox::critical(this, tr("Error"),
                              tr("No se pudieron cargar los datos filtrados:\n%1").arg(query.lastError().text()));
        return;
    }

    modelo->setQuery(std::move(query));

    if(modelo->lastError().isValid()) {
        qDebug() << "Error al establecer modelo con consulta filtrada:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"),
                              tr("Error al procesar los datos filtrados:\n%1").arg(modelo->lastError().text()));
        return;
    }

    qDebug() << "Filtros aplicados. Filas resultantes:" << modelo->rowCount();

    // Actualizar headers después de cambiar los datos
    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Tabla"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Registro ID"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Acción"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Valor Anterior"));
    modelo->setHeaderData(5, Qt::Horizontal, tr("Valor Nuevo"));
    modelo->setHeaderData(6, Qt::Horizontal, tr("Usuario"));
    modelo->setHeaderData(7, Qt::Horizontal, tr("Fecha Acción"));
    modelo->setHeaderData(8, Qt::Horizontal, tr("IP"));
}

void Auditoria::filaSeleccionada(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    if(!current.isValid()) {
        qDebug() << "Selección no válida";
        return;
    }

    int row = current.row();
    qDebug() << "Fila seleccionada:" << row;

    // Actualizar campos con datos de la fila seleccionada
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());
    ui->txtTablaAfectada->setText(modelo->data(modelo->index(row,1)).toString());
    ui->txtRegistroID->setText(modelo->data(modelo->index(row,2)).toString());
    ui->txtAccion->setText(modelo->data(modelo->index(row,3)).toString());
    ui->txtValorAnterior->setPlainText(modelo->data(modelo->index(row,4)).toString());
    ui->txtValorNuevo->setPlainText(modelo->data(modelo->index(row,5)).toString());
    ui->txtUsuario->setText(modelo->data(modelo->index(row,6)).toString());

    // Manejar la fecha de acción
    QVariant fechaData = modelo->data(modelo->index(row,7));
    if (fechaData.isValid()) {
        QDateTime fechaAccion = fechaData.toDateTime();
        if (fechaAccion.isValid()) {
            ui->deFechaAccion->setDateTime(fechaAccion);
        } else {
            // Intentar convertir desde string si toDateTime falla
            QString fechaStr = fechaData.toString();
            QDateTime fechaFromString = QDateTime::fromString(fechaStr, "yyyy-MM-dd HH:mm:ss");
            if (fechaFromString.isValid()) {
                ui->deFechaAccion->setDateTime(fechaFromString);
            } else {
                ui->deFechaAccion->setDateTime(QDateTime::currentDateTime());
                qDebug() << "No se pudo convertir la fecha:" << fechaStr;
            }
        }
    } else {
        ui->deFechaAccion->setDateTime(QDateTime::currentDateTime());
    }

    ui->txtIP->setText(modelo->data(modelo->index(row,8)).toString());

    qDebug() << "Datos de fila cargados en formulario";
}

void Auditoria::on_btnLimpiarFiltros_clicked()
{
    qDebug() << "Botón Limpiar Filtros clickeado";
    limpiarFiltros();
}

void Auditoria::on_btnAplicarFiltros_clicked()
{
    qDebug() << "Botón Aplicar Filtros clickeado";
    aplicarFiltros();
}

void Auditoria::on_btnSalir_clicked()
{
    qDebug() << "Botón Salir clickeado, cerrando ventana...";
    this->close();
}
