#pragma once

#if defined (LOGINSERVER_HAS_DLL) && (LOGINSERVER_HAS_DLL == 1)
#  if defined (LOGINSERVER_BUILD_DLL)
#    define LoginServer_Export __declspec(dllexport)
#  else /* LOGINSERVER_BUILD_DLL */
#    define LoginServer_Export __declspec(dllimport)
#  endif /* LOGINSERVER_BUILD_DLL */
#else /* LOGINSERVER_HAS_DLL == 1 */
#  define LoginServer_Export
#endif /* LOGINSERVER_HAS_DLL == 1 */
