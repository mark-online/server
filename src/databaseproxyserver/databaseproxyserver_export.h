#pragma once

#if defined (DATABASEPROXYSERVER_HAS_DLL) && (DATABASEPROXYSERVER_HAS_DLL == 1)
#  if defined (DATABASEPROXYSERVER_BUILD_DLL)
#    define DatabaseProxyServer_Export __declspec(dllexport)
#  else /* DATABASEPROXYSERVER_BUILD_DLL */
#    define DatabaseProxyServer_Export __declspec(dllimport)
#  endif /* DATABASEPROXYSERVER_BUILD_DLL */
#else /* DATABASEPROXYSERVER_HAS_DLL == 1 */
#  define DatabaseProxyServer_Export
#endif /* DATABASEPROXYSERVER_HAS_DLL == 1 */
