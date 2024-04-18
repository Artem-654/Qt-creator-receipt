#include "mainwindow.h"
#include <QApplication>
#include <QtWidgets>
#include <QImage>
#include <QPainter>

#include <QFile>
#include <QFileDialog>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

enum Type {
    TEXT, IMAGE
};
enum Style {
    BOLD = 0x00001,
    UNDERLINE = 0x00010,
    ITALIC = 0x00100,
};
enum Align {
    LEFT = Qt::AlignLeft, CENTER = Qt::AlignHCenter, RIGHT = Qt::AlignRight
};


class TextToImageWidget : public QWidget {
public:
    TextToImageWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        QFile file;
        file.setFileName(QFileDialog::getOpenFileName(nullptr,"","C:\\","*.json"));

        if(file.open(QIODevice::ReadOnly | QFile::Text))
            doc = QJsonDocument::fromJson(QByteArray(file.readAll()),&docError);
        if (doc.isArray()) {
                mainJsonArray = doc.array();
                qDebug() << "Main JSON array size:" << mainJsonArray.size();
            } else {
                qDebug() << "Error: JSON document is not an array.";
                return;
            }
        file.close();
        currentDateTime = QDateTime::currentDateTime();
    }

    int calculation_of_height()
    {

        return 800;
    }

protected:

    QJsonDocument doc;
    QJsonArray mainJsonArray,jsonArray;
    QJsonParseError docError;
    QDateTime currentDateTime;

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.fillRect(rect(), Qt::white);

        QRect textRect = rect();
        QString text = "";

        int lines = 1;
        double sum = 0;
        QFont font("Verdana", 10);
        painter.setFont(font);
        painter.setPen(Qt::black);

        QJsonArray jsonArray = mainJsonArray.at(0).toArray();
        for (int i = 0; i < jsonArray.size(); i++)
        {
            QJsonValue value = jsonArray.at(i);
            if(value.isObject())
            {
                QJsonObject obj = value.toObject();
                if(i == 0)
                {
                    for(int i = 0; i < lines ; i++)
                        text += '\n';
                    text += obj["name"].toString();
                    text += '\n';
                    text += obj["address"].toString();
                    text += '\n';
                    text += "МАГАЗИН";
                    lines += 5;
                    painter.drawText(textRect, CENTER | Qt::TextWordWrap, text);
                }else
                {
                    text = "";
                    for(int i = 0; i < lines ; i++)
                        text += '\n';
                    text += "ПН: ";
                    text += obj["PN"].toString();
                    text += '\n';
                    text += "ІД: ";
                    text += obj["ID"].toString();
                    text += '\n';
                    text += "Оператор: ";
                    text += obj["operator"].toString();
                    text += '\n';
                    text += "Чек # ";
                    text += obj["Check"].toString();
                    text += '\n';
                    text += "Z: ";
                    text += obj["Z"].toString();
                    text += '\n';
                    text += "Касса: ";
                    text += obj["paydesk"].toString();
                    text += '\n';
                    lines += 6;
                    painter.drawText(textRect, LEFT | Qt::TextWordWrap, text);
                }
            }
        }


        jsonArray = mainJsonArray.at(1).toArray();

        foreach(const QJsonValue &value, jsonArray)
        {
            if(value.isObject())
            {
                QJsonObject obj = value.toObject();

                lines++;
                if(obj["weight_or_count"].toDouble() != 1.000)
                {
                    text = "";
                    for(int i = 0; i < lines ; i++)
                        text += '\n';
                    text += QString::number(obj["weight_or_count"].toDouble(), 'f',3);
                    text += " X";

                    qDebug() << text << '\n';
                    painter.drawText(textRect, LEFT | Qt::TextWordWrap, text);

                    text = "";
                    for(int i = 0; i < lines ; i++)
                        text += '\n';
                    text += QString::number(obj["price_for_weight_or_count"].toDouble(),'f',2);
                    lines++;
                    qDebug() << text << '\n';
                    qDebug() << lines << '\n';

                    painter.drawText(textRect, RIGHT | Qt::TextWordWrap, text);
                }

                text = "";
                for(int i = 0; i < lines ; i++)
                    text += '\n';
                text += obj["product_name"].toString();

                qDebug() << text << '\n';
                painter.drawText(textRect, LEFT | Qt::TextWordWrap, text);

                text = "";
                for(int i = 0; i < lines ; i++)
                    text += '\n';
                double result = obj["price_for_weight_or_count"].toDouble() * obj["weight_or_count"].toDouble();
                sum += result;
                text += QString::number(result, 'f',2);
                text += " A";
                lines++;

                qDebug() << lines << '\n';
                qDebug() << text << '\n';
                painter.drawText(textRect, RIGHT | Qt::TextWordWrap, text);
            }

        }

        text = "";
        for(int i = 0; i < lines ; i++)
            text += '\n';
        text += "------------------------------------------------------------\n\n\n\n";
        text += "------------------------------------------------------------\n\n\n";
        text += "------------------------------------------------------------\n\n\n\n";
        text += QString::number(jsonArray.size());
        text += " АРТИКУЛІВ\n\n";
        text += currentDateTime.toString("dd.MM.yyyy hh:mm:ss");
        text += "\n\n\n\n\n\n\n\n\nФІСКАЛЬНИЙ ЧЕК";
        qDebug() << lines << '\n';
        qDebug() << text << '\n';
        painter.drawText(textRect, CENTER | Qt::TextWordWrap, text);
        jsonArray = mainJsonArray.at(2).toArray();
        if(jsonArray[0].isObject())
        {
            QJsonObject obj = jsonArray[0].toObject();

            text = "";
            for(int i = 0; i < lines ; i++)
                text += '\n';
            text += "\n\nГОТІВКА\nРЕШТА\n\nСУМА\nПДВ А 20.00%\n\nЗаокруглення:\nДо сплати:\n\n\nЧЕК ";
            text += obj["receipt"].toString();
            text += "\n\n\n\n\n\n\n\nФН ";
            text += obj["FN"].toString();
            painter.drawText(textRect, LEFT | Qt::TextWordWrap, text);

            text = "";
            for(int i = 0; i < lines ; i++)
                text += '\n';
            text += "\n\n";
            text += QString::number(obj["cash"].toDouble(),'f',2);
            text += '\n';
            double rounded = (obj["cash"].toDouble() - sum) - qRound(obj["cash"].toDouble() - sum);
            text += QString::number(obj["cash"].toDouble() - sum, 'f',2);
            text += "\n\n";
            text += QString::number(sum,'f',2);
            text += '\n';
            text += QString::number(sum-(sum / 1.2),'f',2);
            text += "\n\n";
            text += QString::number(rounded,'f',2);
            text += '\n';
            text += QString::number(sum + rounded,'f',2);
            text += "\n\n\n";
            text += "ОНЛАЙН\n\n\n\n\n\n\n\n";
            text += obj["?"].toString();
            painter.drawText(textRect, RIGHT | Qt::TextWordWrap, text);
        }

        QImage image = QImage(size(), QImage::Format_ARGB32);
        QPainter imagePainter(&image);
        render(&imagePainter);
        imagePainter.end();

        image.save("text_image.png");
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

        TextToImageWidget widget;
        widget.resize(400, widget.calculation_of_height());
        widget.show();

        return app.exec();

}
