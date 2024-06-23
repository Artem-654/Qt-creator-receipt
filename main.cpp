#include "mainwindow.h"
#include <vector>
#include <memory>
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
    TEXT, PRODUCT
};
enum Style {
    BOLD = 0x00001,
    UNDERLINE = 0x00010,
    ITALIC = 0x00100,
};
enum Align {
    LEFT = Qt::AlignLeft,
    CENTER = Qt::AlignHCenter,
    RIGHT = Qt::AlignRight
};


class Item {
protected:
    int size = 1;
    Type type;
    Align align;
    QString text;

    Align charToAlign(char alignChar) {
        switch (alignChar) {
            case 'L': return LEFT;
            case 'C': return CENTER;
            case 'R': return RIGHT;
            default: throw std::invalid_argument("Invalid align value");
        }

    }

public:
    Item(QJsonObject obj)
        : type(static_cast<Type>(obj["type"].toInt())),
          align(charToAlign(obj["align"].toString().at(0).toLatin1())),
          text(obj["text"].toString()){}

    virtual void printItem(int &lines, QRect &textRect, QPainter &painter, QFont font) = 0;
};

class TextItem : public Item {
protected:
    Style style;

    Style arrayToStyle(const QJsonArray &styleArray) {
        int styleBitmask = 0;
            for (const auto &value : styleArray) {
                QString styleStr = value.toString().toUpper();
                if (styleStr == "BOLD") {
                    styleBitmask |= BOLD;
                } else if (styleStr == "UNDERLINE") {
                    styleBitmask |= UNDERLINE;
                } else if (styleStr == "ITALIC") {
                    styleBitmask |= ITALIC;
                }
            }
            return static_cast<Style>(styleBitmask);
    }

    void applyStyle(QPainter &painter, QFont &font) {
        if (style & BOLD) {
            font.setBold(true);
        }
        else
        {
            font.setBold(false);
        }
        if (style & ITALIC) {
            font.setItalic(true);
        }
        else
        {
            font.setItalic(false);
        }
        if (style & UNDERLINE) {
            font.setUnderline(true);
        }
        else
        {
            font.setUnderline(false);
        }

        painter.setFont(font);
    }
public:
    TextItem(QJsonObject obj) : Item(obj), style(arrayToStyle(obj["style"].toArray())){
        for(int i = 0, textSizePerLine = 0;i<text.size();i++, textSizePerLine++)
        {
            if(text.at(i) == '\n')
            {
                size++;
                textSizePerLine = 0;
                continue;
            }else if(!(textSizePerLine%49))
            {
                text.insert(i,'\n');
                size++;
            }
        }
    }
    void printItem(int &lines, QRect &textRect, QPainter &painter, QFont font) override
    {
        QString tempText;
        for(int i = 0;i<lines;i++)
            tempText += '\n';

        tempText += text;
        applyStyle(painter, font);
        painter.drawText(textRect, align | Qt::TextWordWrap, tempText);

        lines += size;
    }
};

class ProductItem : public TextItem {
protected:
    float price;
    float weight;
public:
    ProductItem(QJsonObject obj) : TextItem(obj), price(obj["price"].toDouble()),
                                   weight(obj["weight"].toDouble()){}
    void printItem(int &lines, QRect &textRect, QPainter &painter, QFont font) override
    {
        QString tempText;
        lines++;
        if(weight != 1.000)
        {
            for(int i = 0;i<lines;i++)
                tempText += '\n';
            tempText += QString::number(weight, 'f',3);
            tempText += " X";
            painter.drawText(textRect, LEFT | Qt::TextWordWrap, tempText);
            tempText = "";

            for(int i = 0;i<lines;i++)
                tempText += '\n';
            tempText += QString::number(price,'f',2);
            tempText += '\n';
            painter.drawText(textRect, RIGHT | Qt::TextWordWrap, tempText);
            tempText = "";
        }

        for(int i = 0;i<lines;i++)
            tempText += '\n';
        tempText += text;
        applyStyle(painter, font);
        painter.drawText(textRect, align | Qt::TextWordWrap, tempText);

        tempText = "\n";
        for(int i = 0;i<lines;i++)
            tempText += '\n';
        tempText += QString::number(price * weight,'f',2);
        tempText += " A";
        painter.drawText(textRect, RIGHT | Qt::TextWordWrap, tempText);

        lines += size;
    }
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
                jsonArray = doc.array();
                qDebug() << "Main JSON array size:" << jsonArray.size();
            } else {
                qDebug() << "Error: JSON document is not an array." << docError.errorString();
                return;
            }
        file.close();
        currentDateTime = QDateTime::currentDateTime();
        for (int i = 0; i < jsonArray.size(); i++)
        {
            QJsonValue value = jsonArray.at(i);
            if(value.isObject())
            {
                QJsonObject obj = value.toObject();
                switch(obj["type"].toInt())
                {
                case 0:
                    arrayItems.push_back(std::unique_ptr<TextItem>(new TextItem(obj)));
                    break;
                case 1:
                    arrayItems.push_back(std::unique_ptr<ProductItem>(new ProductItem(obj)));
                    break;
                }
            }
        }
    }
    static int get_hight()
    {
        return lines * 15;
    }
    static void add_to_lines()
    {
        lines++;
    }
protected:

    static int lines;
    std::vector<std::unique_ptr<Item>> arrayItems;
    QJsonDocument doc;
    QJsonArray jsonArray;
    QJsonParseError docError;
    QDateTime currentDateTime;

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.fillRect(rect(), Qt::white);

        QRect textRect = rect();

        QFont font("Verdana", 10);
        painter.setFont(font);
        painter.setPen(Qt::black);

        for (int i = 0; i < jsonArray.size(); i++)
        {
            arrayItems[i]->printItem(lines, textRect, painter, font);
            //painter.drawText(textRect, CENTER | Qt::TextWordWrap, text);
        }
        lines = 0;
        QImage image = QImage(size(), QImage::Format_ARGB32);
        QPainter imagePainter(&image);
        render(&imagePainter);
        qDebug() << QString::number(TextToImageWidget::height());
        imagePainter.end();

        image.save("text_image.png");
    }
};
int TextToImageWidget::lines = 1;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

        TextToImageWidget widget;
        widget.resize(400, 720);
        widget.show();

        return app.exec();

}
