#ifndef HYDRA_EXPORT_H
#define HYDRA_EXPORT_H

#include <QObject>

#ifdef Q_OS_WIN
#ifdef LIBHYDRA_BUILD
#define HYDRAEXPORT __declspec(dllexport)
#else
#define HYDRAEXPORT __declspec(dllimport)
#endif
#else
#define HYDRAEXPORT
#endif

#endif // HYDRA_EXPORT_H

