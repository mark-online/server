#pragma once

#if defined (COMMUNITYSERVER_HAS_DLL) && (COMMUNITYSERVER_HAS_DLL == 1)
#  if defined (COMMUNITYSERVER_BUILD_DLL)
#    define CommunityServer_Export __declspec(dllexport)
#  else /* COMMUNITYSERVER_BUILD_DLL */
#    define CommunityServer_Export __declspec(dllimport)
#  endif /* COMMUNITYSERVER_BUILD_DLL */
#else /* COMMUNITYSERVER_HAS_DLL == 1 */
#  define CommunityServer_Export
#endif /* COMMUNITYSERVER_HAS_DLL == 1 */
