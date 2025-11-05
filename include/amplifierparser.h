#ifndef AMPLIFIERPARSER_H
#define AMPLIFIERPARSER_H

#include <QString>
#include <QStringList>
#include <QRegularExpression>

struct AmplifierInfo
{
    QString name;
    QString id;
    QStringList available_channels;
    QStringList available_samplings;
};

class AmplifierParser
{
public:
    static QList<AmplifierInfo> Parse(const QByteArray& output)
    {
        QList<AmplifierInfo> amplifiers;
        QString text = QString::fromUtf8(output);
        QStringList lines = text.split('\n', Qt::SkipEmptyParts);

        AmplifierInfo current_amp;
        bool in_amplifier = false;
        bool in_channels = false;
        bool in_sampling_rates = false;

        for(const QString& line : std::as_const(lines))
        {
            QString trimmed = line.trimmed();

            // Check if we are starting new ampl
            if(trimmed.startsWith("* "))
            {
                if(in_amplifier && !current_amp.name.isEmpty())
                {
                    amplifiers.append(current_amp);
                }

                // Start new one
                current_amp = AmplifierInfo();
                current_amp.name = trimmed.mid(2).trimmed(); // remove * prefix
                in_amplifier = true;
                in_channels = false;
                in_sampling_rates = false;
            }
            else if(trimmed.startsWith("id:"))
            {
                static QRegularExpression id_regex(R"(id:\s*"([^"]+)\")");
                QRegularExpressionMatch match = id_regex.match(trimmed);
                if(match.hasMatch())
                {
                    current_amp.id = match.captured(1);
                }
            }
            else if(trimmed.contains("available channels:"))
            {
                in_channels = true;
                in_sampling_rates = false;
            }
            else if(trimmed.contains("available sampling rates:"))
            {
                in_channels = false;
                in_sampling_rates = true;
            }
            else if(in_channels && !trimmed.isEmpty() && !trimmed.contains("available"))
            {
                static QRegularExpression re("\\s+");
                QStringList channels = trimmed.split(re, Qt::SkipEmptyParts);
                current_amp.available_channels.append(channels);
            }
            else if(in_sampling_rates && !trimmed.isEmpty() && !trimmed.contains("available"))
            {
                static QRegularExpression re("\\s+");
                QStringList rates = trimmed.split(re, Qt::SkipEmptyParts);
                current_amp.available_samplings.append(rates);
            }
        }

        // Last amp
        if(in_amplifier && !current_amp.name.isEmpty())
        {
            amplifiers.append(current_amp);
        }

        return amplifiers;
    }
};

#endif // AMPLIFIERPARSER_H
