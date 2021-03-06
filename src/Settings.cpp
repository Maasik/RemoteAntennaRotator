#include <QStringList>
#include <QVariant>
#include <QLoggingCategory>
#include "Settings.h"

static const QString TCP_UART_CONTROLLER = "tcp_uart_controller";
static const QString TCP_UART_CONTROLLER_ID = "tcp_uart_controller_id";
static const QString ENCODER = "encoder";
static const QString ROTATOR = "rotator";
static const QString NAME = "name";
static const QString TYPE = "type";
static const QString IP = "ip";
static const QString ID = "id";
static const QString PORT = "port";
static const QString MAX_WAITE_DATA = "maxWaiteData";
static const QString ADDRESS = "address";
static const QString ZERO_ANGLE = "zeroAngle";
static const QString SEPARATOR = "/";

Settings* Settings::m_instance=0x0;

Settings::Settings()
{

}

Settings::~Settings()
{

}

Settings *Settings::getInstance()
{
  if(m_instance==0x0)
  {
    m_instance = new Settings();
    if(!m_instance->loadSettings())
    {
      delete m_instance;
      m_instance=0x0;
    }
  }
  return m_instance;
}

void Settings::resetInstance()
{
  if(m_instance)
  {
    delete m_instance;
    m_instance=0x0;
  }
}

bool Settings::loadSettings()
{
  qInfo() << "Start loadSettings()";
  bool ok(true);
  QSettings settings("Config/settings.ini", QSettings::IniFormat);
  if(!loadTcpModule(settings, TCP_UART_CONTROLLER))
  {
    qCritical()  << "Can't load tcp uart module list";
    return false;
  }

  QStringList groups = settings.childGroups();
  foreach (QString groupName, groups)
  {
    if(groupName.contains(ENCODER) && !loadEncoder(settings, groupName))
    {
      ok = false;
      break;
    }
    else if(groupName.contains(ROTATOR) && !loadRotator(settings, groupName))
    {
      ok = false;
      break;
    }
  }

  qInfo() << "End loadSettings() " << (ok?"":"load error");
  return ok;
}

const QVector<Encoder *> &Settings::getEncoderList() const
{
  return m_encoderList;
}

const QVector<TcpUartModule *> &Settings::getTcpModuleList() const
{
  return m_tcpModuleList;
}

bool Settings::loadEncoder(const QSettings &settings, const QString &path)
{
  QString id = settings.value(path+SEPARATOR+TCP_UART_CONTROLLER_ID).toString();
  if(id.isEmpty())
  {
    qCritical() << path << SEPARATOR << TCP_UART_CONTROLLER_ID << "is empty";
    return false;
  }
  QString name = settings.value(path+SEPARATOR+NAME).toString();
  if(id.isEmpty())
  {
    qCritical() << path << SEPARATOR << NAME << "is empty";
    return false;
  }
  QString address = settings.value(path+SEPARATOR+ADDRESS).toString();
  int zeroAngle = settings.value(path+SEPARATOR+ZERO_ANGLE, 0).toInt();

  if(address.isEmpty())
  {
    qCritical() << path << SEPARATOR << ADDRESS << "is empty";
    return false;
  }
  TcpUartModule* tcpModule =0x0;
  foreach (TcpUartModule* m, m_tcpModuleList)
  {
    if(m->id().contains(id))
    {
      tcpModule = m;
      break;
    }
  }
  if(tcpModule)
  {
    m_encoderList.append(new Encoder(name, QByteArray::fromHex(address.toLatin1()), zeroAngle, tcpModule));
  }
  else
  {
    qCritical() << "Can't find tcp uart module for " << path;
    return false;
  }
  return true;
}

bool Settings::loadRotator(const QSettings &settings, const QString &path)
{
  QString id = settings.value(path+SEPARATOR+TCP_UART_CONTROLLER_ID).toString();
  if(id.isEmpty())
  {
    qCritical() << path << SEPARATOR << TCP_UART_CONTROLLER_ID << "is empty";
    return false;
  }

  QString address = settings.value(path+SEPARATOR+ADDRESS).toString();

  if(address.isEmpty())
  {
    qCritical() << path << SEPARATOR << ADDRESS << "is empty";
    return false;
  }

  TcpUartModule* tcpModule =0x0;
  foreach (TcpUartModule* m, m_tcpModuleList)
  {
    if(m->id().contains(id))
    {
      tcpModule = m;
      break;
    }
  }
  if(tcpModule)
  {
    m_rotatorList.append(new Rotator(QByteArray::fromHex(address.toLatin1()), tcpModule));
  }
  else
  {
    qCritical() << "Can't find tcp uart module for " << path;
    return false;
  }
  return true;
}

bool Settings::loadTcpModule(const QSettings &settings, const QString &path)
{
  QString id = settings.value(path+SEPARATOR+ID).toString();
  if(id.isEmpty())
  {
    qCritical() << path << SEPARATOR << ID << "is empty";
    return false;
  }
  QString ip = settings.value(path+SEPARATOR+IP).toString();
  if(ip.isEmpty())
  {
    qCritical() << path << SEPARATOR << IP << " is empty";
    return false;
  }
  int port = settings.value(path+SEPARATOR+PORT).toInt();
  if(port == 0)
  {
    qCritical() << path << SEPARATOR << PORT << " is empty";
    return false;
  }
  int maxWaiteData = settings.value(path+SEPARATOR+MAX_WAITE_DATA).toInt();
  if(maxWaiteData==0)
  {
    qCritical() << path << SEPARATOR << MAX_WAITE_DATA << " is empty";
    return false;
  }
  m_tcpModuleList.append(new TcpUartModule(id,ip, port, maxWaiteData));
  return true;
}

const QVector<Rotator *> Settings::getRotatorList() const
{
  return m_rotatorList;
}

int Settings::getAngleShiftForEncoder(unsigned short address) const
{
  foreach (Encoder* e, m_encoderList) {
    if(address == e->getAddress())
    {
      return e->getAngleShift();
    }
  }
}
