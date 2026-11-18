#include "ajustes.h"
#include "ui_ajustes.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QModelIndex>

Ajustes::Ajustes(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Ajustes),
    modelo(new QSqlQueryModel(this))
{
    ui->setupUi(this);

    // ------------------------------ Estilo cian moderno ------------------------------
    this->setStyleSheet("background-color: #f0fdff;"); // Fondo cian claro muy suave

    QString lineEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtID->setStyleSheet(lineEditStyle);
    ui->txtObservaciones->setStyleSheet(lineEditStyle);

    QString labelStyle = "color: #00838f; font-weight: bold; font-size: 14px;";
    ui->labelID->setStyleSheet(labelStyle);
    ui->labelTipoAjuste->setStyleSheet(labelStyle);
    ui->labelFechaAjuste->setStyleSheet(labelStyle);
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
    ui->cbTipoAjuste->setStyleSheet(comboStyle);
    ui->cbUsuario->setStyleSheet(comboStyle);

    // Estilo para QDateTimeEdit
    QString dateStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->dtFechaAjuste->setStyleSheet(dateStyle);

    ui->txtID->setEnabled(false);

    // Configurar formato de fecha
    ui->dtFechaAjuste->setDisplayFormat("yyyy-MM-dd HH:mm");
    ui->dtFechaAjuste->setDateTime(QDateTime::currentDateTime());

    // Configurar QTextEdit para observaciones
    QString textEditStyle = "background-color: #ffffff; color: #000000; border: 1px solid #00bcd4; border-radius: 6px; padding: 6px; font-size: 14px;";
    ui->txtObservaciones->setStyleSheet(textEditStyle);

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

    ui->tablaAjustes->setStyleSheet(tablaStyle);

    // Primero cargamos combos y luego datos
    cargarCombos();
    cargarDatos();

    // Conectar la señal después de setModel
    if (ui->tablaAjustes->selectionModel()) {
        connect(ui->tablaAjustes->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                &Ajustes::filaSeleccionada);
    }
}

Ajustes::~Ajustes()
{
    delete ui;
}

void Ajustes::cargarCombos()
{
    // Cargar tipos de ajuste
    ui->cbTipoAjuste->clear();
    QSqlQuery query;
    if (!query.exec("SELECT id, nombre FROM ajustes_tipo ORDER BY nombre")) {
        qDebug() << "Error cargando tipos de ajuste:" << query.lastError().text();
        QMessageBox::warning(this, tr("Error"), tr("No se pudieron cargar los tipos de ajuste:\n%1").arg(query.lastError().text()));
        return;
    }

    while(query.next()) {
        int id = query.value(0).toInt();
        QString nombre = query.value(1).toString();
        ui->cbTipoAjuste->addItem(nombre, id);
    }

    // Cargar usuarios activos - CORREGIDO: usando username en lugar de nombre
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

void Ajustes::cargarDatos()
{
    // CORREGIDO: Usando username en lugar de nombre
    QString sql = "SELECT a.id, "
                  "at.nombre AS tipo_ajuste, "
                  "TO_CHAR(a.fecha_ajuste, 'YYYY-MM-DD HH24:MI') AS fecha_ajuste, "
                  "COALESCE(a.observaciones, 'Sin observaciones') AS observaciones, "
                  "u.username || ' (' || u.rol || ')' AS usuario "
                  "FROM ajustes a "
                  "LEFT JOIN ajustes_tipo at ON a.ajuste_tipo_id = at.id "
                  "LEFT JOIN usuario u ON a.usuario_id = u.id "
                  "ORDER BY a.fecha_ajuste DESC";

    modelo->setQuery(sql);

    if(modelo->lastError().isValid()) {
        qDebug() << "Error en consulta de datos:" << modelo->lastError().text();
        QMessageBox::critical(this, tr("Error"), tr("No se pudieron cargar los datos:\n%1").arg(modelo->lastError().text()));
        return;
    }

    modelo->setHeaderData(0, Qt::Horizontal, tr("ID"));
    modelo->setHeaderData(1, Qt::Horizontal, tr("Tipo Ajuste"));
    modelo->setHeaderData(2, Qt::Horizontal, tr("Fecha Ajuste"));
    modelo->setHeaderData(3, Qt::Horizontal, tr("Observaciones"));
    modelo->setHeaderData(4, Qt::Horizontal, tr("Usuario"));

    ui->tablaAjustes->setModel(modelo);
    ui->tablaAjustes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tablaAjustes->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tablaAjustes->horizontalHeader()->setStretchLastSection(true);
    ui->tablaAjustes->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Ajustar el ancho de las columnas
    ui->tablaAjustes->setColumnWidth(0, 50);   // ID
    ui->tablaAjustes->setColumnWidth(1, 150);  // Tipo Ajuste
    ui->tablaAjustes->setColumnWidth(2, 150);  // Fecha Ajuste
    ui->tablaAjustes->setColumnWidth(3, 250);  // Observaciones
    ui->tablaAjustes->setColumnWidth(4, 150);  // Usuario

    limpiarCampos();
}

void Ajustes::limpiarCampos()
{
    ui->txtID->clear();
    ui->txtObservaciones->clear();

    if(ui->cbTipoAjuste->count() > 0)
        ui->cbTipoAjuste->setCurrentIndex(0);
    if(ui->cbUsuario->count() > 0)
        ui->cbUsuario->setCurrentIndex(0);

    ui->dtFechaAjuste->setDateTime(QDateTime::currentDateTime());

    ui->cbTipoAjuste->setFocus();
}

void Ajustes::on_btnNuevo_clicked()
{
    limpiarCampos();
}

void Ajustes::on_btnGuardar_clicked()
{
    int tipoAjusteId = ui->cbTipoAjuste->currentData().toInt();
    QDateTime fechaAjuste = ui->dtFechaAjuste->dateTime();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(tipoAjusteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un tipo de ajuste."));
        ui->cbTipoAjuste->setFocus();
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        ui->cbUsuario->setFocus();
        return;
    }

    if(fechaAjuste > QDateTime::currentDateTime()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha del ajuste no puede ser futura."));
        ui->dtFechaAjuste->setFocus();
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO ajustes (ajuste_tipo_id, fecha_ajuste, observaciones, usuario_id) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(tipoAjusteId);
    query.addBindValue(fechaAjuste);
    query.addBindValue(observaciones.isEmpty() ? QVariant() : QVariant(observaciones));
    query.addBindValue(usuarioId);

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al guardar"), tr("No se pudo guardar el ajuste:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Guardado"), tr("Ajuste guardado correctamente."));
}

void Ajustes::on_btnModificar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    int tipoAjusteId = ui->cbTipoAjuste->currentData().toInt();
    QDateTime fechaAjuste = ui->dtFechaAjuste->dateTime();
    QString observaciones = ui->txtObservaciones->toPlainText().trimmed();
    int usuarioId = ui->cbUsuario->currentData().toInt();

    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un ajuste para modificar."));
        return;
    }

    if(tipoAjusteId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un tipo de ajuste."));
        ui->cbTipoAjuste->setFocus();
        return;
    }

    if(usuarioId == 0) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un usuario."));
        ui->cbUsuario->setFocus();
        return;
    }

    if(fechaAjuste > QDateTime::currentDateTime()) {
        QMessageBox::warning(this, tr("Atención"), tr("La fecha del ajuste no puede ser futura."));
        ui->dtFechaAjuste->setFocus();
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE ajustes SET ajuste_tipo_id=?, fecha_ajuste=?, observaciones=?, usuario_id=? WHERE id=?");
    query.addBindValue(tipoAjusteId);
    query.addBindValue(fechaAjuste);
    query.addBindValue(observaciones.isEmpty() ? QVariant() : QVariant(observaciones));
    query.addBindValue(usuarioId);
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al modificar"), tr("No se pudo modificar el ajuste:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Modificado"), tr("Ajuste modificado correctamente."));
}

void Ajustes::on_btnEliminar_clicked()
{
    QString id = ui->txtID->text().trimmed();
    if(id.isEmpty()) {
        QMessageBox::warning(this, tr("Atención"), tr("Seleccione un ajuste para eliminar."));
        return;
    }

    // COMENTADO: Verificación de tabla ajustes_detalle que puede no existir
    /*
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM ajustes_detalle WHERE ajuste_id = ?");
    checkQuery.addBindValue(id.toInt());
    if(checkQuery.exec() && checkQuery.next()) {
        int count = checkQuery.value(0).toInt();
        if(count > 0) {
            QMessageBox::warning(this, tr("No se puede eliminar"),
                                tr("No se puede eliminar este ajuste porque tiene %1 detalle(s) asociado(s).").arg(count));
            return;
        }
    }
    */

    if(QMessageBox::question(this, tr("Confirmar eliminación"),
                              tr("¿Estás seguro que deseas eliminar este ajuste?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM ajustes WHERE id=?");
    query.addBindValue(id.toInt());

    if(!query.exec()) {
        QMessageBox::critical(this, tr("Error al eliminar"), tr("No se pudo eliminar el ajuste:\n%1").arg(query.lastError().text()));
        return;
    }

    cargarDatos();
    QMessageBox::information(this, tr("Eliminado"), tr("Ajuste eliminado correctamente."));
}

void Ajustes::filaSeleccionada(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if(!current.isValid()) {
        limpiarCampos();
        return;
    }

    int row = current.row();
    ui->txtID->setText(modelo->data(modelo->index(row,0)).toString());

    // Obtener los datos directamente de la base de datos para una búsqueda precisa
    int idAjuste = modelo->data(modelo->index(row,0)).toInt();

    QSqlQuery query;
    query.prepare("SELECT ajuste_tipo_id, fecha_ajuste, observaciones, usuario_id "
                  "FROM ajustes WHERE id = ?");
    query.addBindValue(idAjuste);

    if(query.exec() && query.next()) {
        int tipoAjusteId = query.value(0).toInt();
        QDateTime fechaAjuste = query.value(1).toDateTime();
        QString observaciones = query.value(2).toString();
        int usuarioId = query.value(3).toInt();

        // Buscar por ID en lugar de por texto
        int indexTipoAjuste = ui->cbTipoAjuste->findData(tipoAjusteId);
        if(indexTipoAjuste >= 0) {
            ui->cbTipoAjuste->setCurrentIndex(indexTipoAjuste);
        }

        int indexUsuario = ui->cbUsuario->findData(usuarioId);
        if(indexUsuario >= 0) {
            ui->cbUsuario->setCurrentIndex(indexUsuario);
        }

        if(fechaAjuste.isValid()) {
            ui->dtFechaAjuste->setDateTime(fechaAjuste);
        }

        ui->txtObservaciones->setPlainText(observaciones);
    } else {
        qDebug() << "Error al cargar datos del ajuste:" << query.lastError().text();
    }
}

void Ajustes::on_btnSalir_clicked()
{
    close();
}
