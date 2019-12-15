#pragma once

#if defined (ZONESERVER_HAS_DLL) && (ZONESERVER_HAS_DLL == 1)
#  if defined (ZONESERVER_BUILD_DLL)
#    define ZoneServer_Export __declspec(dllexport)
#  else /* ZONESERVER_BUILD_DLL */
#    define ZoneServer_Export __declspec(dllimport)
#  endif /* ZONESERVER_BUILD_DLL */
#else /* ZONESERVER_HAS_DLL == 1 */
#  define ZoneServer_Export
#endif /* ZONESERVER_HAS_DLL == 1 */
