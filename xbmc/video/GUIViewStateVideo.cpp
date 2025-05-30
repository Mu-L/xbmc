/*
 *  Copyright (C) 2016-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "GUIViewStateVideo.h"

#include "FileItem.h"
#include "FileItemList.h"
#include "ServiceBroker.h"
#include "VideoDatabase.h"
#include "filesystem/Directory.h"
#include "filesystem/VideoDatabaseDirectory.h"
#include "filesystem/VideoDatabaseDirectory/DirectoryNode.h"
#include "filesystem/VideoDatabaseDirectory/QueryParams.h"
#include "guilib/WindowIDs.h"
#include "playlists/PlayListFileItemClassify.h"
#include "playlists/PlayListTypes.h"
#include "settings/MediaSettings.h"
#include "settings/MediaSourceSettings.h"
#include "settings/Settings.h"
#include "settings/SettingsComponent.h"
#include "utils/FileExtensionProvider.h"
#include "utils/SortUtils.h"
#include "video/VideoFileItemClassify.h"
#include "view/ViewStateSettings.h"

using namespace KODI;
using namespace XFILE;
using namespace VIDEODATABASEDIRECTORY;

std::string CGUIViewStateWindowVideo::GetLockType()
{
  return "video";
}

std::string CGUIViewStateWindowVideo::GetExtensions()
{
  return CServiceBroker::GetFileExtensionProvider().GetVideoExtensions();
}

PLAYLIST::Id CGUIViewStateWindowVideo::GetPlaylist() const
{
  return PLAYLIST::Id::TYPE_VIDEO;
}

std::vector<CMediaSource>& CGUIViewStateWindowVideo::GetSources()
{
  AddLiveTVSources();
  return CGUIViewState::GetSources();
}

bool CGUIViewStateWindowVideo::AutoPlayNextItem()
{
  return AutoPlayNextVideoItem();
}

/***************************/

CGUIViewStateWindowVideoNav::CGUIViewStateWindowVideoNav(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  SortAttribute sortAttributes = SortAttributeNone;
  if (CServiceBroker::GetSettingsComponent()->GetSettings()->GetBool(CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING))
    sortAttributes = SortAttributeIgnoreArticle;

  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, Size | Label, empty
    SetSortMethod(SortByNone);

    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SortOrderNone);
  }
  else if (VIDEO::IsVideoDb(items))
  {
    NodeType nodeType = CVideoDatabaseDirectory::GetDirectoryChildType(items.GetPath());
    CQueryParams params;
    CVideoDatabaseDirectory::GetQueryParams(items.GetPath(),params);

    switch (nodeType)
    {
      case NodeType::MOVIES_OVERVIEW:
      case NodeType::TVSHOWS_OVERVIEW:
      case NodeType::MUSICVIDEOS_OVERVIEW:
      case NodeType::OVERVIEW:
      {
        AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, Size | Label, empty

        SetSortMethod(SortByNone);

        SetViewAsControl(DEFAULT_VIEW_LIST);

        SetSortOrder(SortOrderNone);
      }
      break;
      case NodeType::DIRECTOR:
      case NodeType::ACTOR:
      {
        AddSortMethod(SortByLabel, 551, LABEL_MASKS("%T", "%R", "%L", ""));  // Title, Rating | Label, empty
        AddSortMethod(SortByRelevance, 38026, LABEL_MASKS("%T", "%c", "%L", "%c"));  // Title, Actor's appearances (Relevance) | Label, Actor's appearances (Relevance)
        SetSortMethod(SortByLabel);

        const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavactors");
        SetViewAsControl(viewState->m_viewMode);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
      }
      break;
      case NodeType::YEAR:
      {
        AddSortMethod(SortByLabel, 562, LABEL_MASKS("%T", "%R", "%L", ""));  // Title, Rating | Label, empty
        SetSortMethod(SortByLabel);

        const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavyears");
        SetViewAsControl(viewState->m_viewMode);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
      }
      break;
      case NodeType::SEASONS:
      {
        AddSortMethod(SortBySeason, 20373,
                      LABEL_MASKS("%L", "", "%L", "")); // Label, empty | Label, empty
        SetSortMethod(SortBySeason);

        const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavseasons");
        SetViewAsControl(viewState->m_viewMode);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
      }
      break;
      case NodeType::TITLE_TVSHOWS:
      case NodeType::INPROGRESS_TVSHOWS:
      {
        AddSortMethod(SortBySortTitle, sortAttributes, 556, LABEL_MASKS("%T", "%M", "%T", "%M"));  // Title, #Episodes | Title, #Episodes
        AddSortMethod(SortByOriginalTitle, sortAttributes, 20376,
                      LABEL_MASKS("%T", "%M", "%T", "%M")); // Title, #Episodes | Title, #Episodes

        AddSortMethod(SortByNumberOfEpisodes, sortAttributes, 20360,
                      LABEL_MASKS("%L", "%M", "%L", "%M")); // Label, #Episodes | Label, #Episodes
        AddSortMethod(
            SortByLastPlayed, sortAttributes, 568,
            LABEL_MASKS("%T", "%p", "%T", "%p")); // Title, #Last played | Title, #Last played
        AddSortMethod(SortByDateAdded, sortAttributes, 570,
                      LABEL_MASKS("%T", "%a", "%T", "%a")); // Title, DateAdded | Title, DateAdded
        AddSortMethod(SortByYear, sortAttributes, 562,
                      LABEL_MASKS("%L", "%Y", "%L", "%Y")); // Label, Year | Label, Year
        AddSortMethod(SortByRating, sortAttributes, 563,
                      LABEL_MASKS("%T", "%R", "%T", "%R")); // Title, Rating | Title, Rating
        AddSortMethod(SortByUserRating, sortAttributes, 38018,
                      LABEL_MASKS("%T", "%r", "%T", "%r")); // Title, Userrating | Title, Userrating
        SetSortMethod(SortByLabel);

        const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavtvshows");
        SetViewAsControl(viewState->m_viewMode);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
      }
      break;
      case NodeType::MUSICVIDEOS_ALBUM:
      case NodeType::GENRE:
      case NodeType::COUNTRY:
      case NodeType::STUDIO:
      {
        AddSortMethod(SortByLabel, 551, LABEL_MASKS("%T", "%R", "%L", ""));  // Title, Rating | Label, empty
        SetSortMethod(SortByLabel);

        const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavgenres");
        SetViewAsControl(viewState->m_viewMode);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
      }
      break;
      case NodeType::SETS:
      {
        AddSortMethod(SortByLabel, sortAttributes, 551, LABEL_MASKS("%T","%R", "%T","%R"));  // Title, Rating | Title, Rating

        AddSortMethod(SortByYear, 562, LABEL_MASKS("%T", "%Y", "%T", "%Y"));  // Title, Year | Title, Year
        AddSortMethod(SortByRating, 563, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
        AddSortMethod(SortByDateAdded, 570, LABEL_MASKS("%T", "%a", "%T", "%a"));  // Title, DateAdded | Title, DateAdded
        AddSortMethod(SortByPlaycount, 567,
                      LABEL_MASKS("%T", "%V", "%T", "%V")); // Title, Playcount | Title, Playcount

        SetSortMethod(SortByLabel);

        const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavgenres");
        SetViewAsControl(viewState->m_viewMode);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
      }
      break;
      case NodeType::TAGS:
      case NodeType::VIDEOVERSIONS:
      {
        AddSortMethod(SortByLabel, sortAttributes, 551,
                      LABEL_MASKS("%T", "", "%T", "")); // Title, empty | Title, empty
        SetSortMethod(SortByLabel);

        const CViewState* viewState = CViewStateSettings::GetInstance().Get("videonavgenres");
        SetViewAsControl(viewState->m_viewMode);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
      }
      break;
      case NodeType::EPISODES:
      {
        if (params.GetSeason() > -1)
        {
          AddSortMethod(SortByEpisodeNumber, 20359, LABEL_MASKS("%E. %T","%R"));  // Episode. Title, Rating | empty, empty
          AddSortMethod(SortByRating, 563, LABEL_MASKS("%E. %T", "%R"));  // Episode. Title, Rating | empty, empty
          AddSortMethod(SortByUserRating, 38018, LABEL_MASKS("%E. %T", "%r"));  // Episode. Title, Userrating | empty, empty
          AddSortMethod(SortByMPAA, 20074, LABEL_MASKS("%E. %T", "%O"));  // Episode. Title, MPAA | empty, empty
          AddSortMethod(SortByProductionCode, 20368, LABEL_MASKS("%E. %T","%P", "%E. %T","%P"));  // Episode. Title, ProductionCode | Episode. Title, ProductionCode
          AddSortMethod(SortByDate, 552, LABEL_MASKS("%E. %T","%J","%E. %T","%J"));  // Episode. Title, Date | Episode. Title, Date
          AddSortMethod(SortByPlaycount, 567,
                        LABEL_MASKS("%E. %T", "%V")); // Episode. Title, Playcount | empty, empty
        }
        else
        {
          AddSortMethod(SortByEpisodeNumber, 20359, LABEL_MASKS("%H. %T","%R"));  // Order. Title, Rating | empty, empty
          AddSortMethod(SortByRating, 563, LABEL_MASKS("%H. %T", "%R"));  // Order. Title, Rating | empty, empty
          AddSortMethod(SortByUserRating, 38018, LABEL_MASKS("%H. %T", "%r"));  // Order. Title, Userrating | empty, empty
          AddSortMethod(SortByMPAA, 20074, LABEL_MASKS("%H. %T", "%O"));  // Order. Title, MPAA | empty, empty
          AddSortMethod(SortByProductionCode, 20368, LABEL_MASKS("%H. %T","%P", "%H. %T","%P"));  // Order. Title, ProductionCode | Episode. Title, ProductionCode
          AddSortMethod(SortByDate, 552, LABEL_MASKS("%H. %T","%J","%H. %T","%J"));  // Order. Title, Date | Episode. Title, Date
          AddSortMethod(SortByPlaycount, 567,
                        LABEL_MASKS("%H. %T", "%V")); // Order. Title, Playcount | empty, empty
        }
        AddSortMethod(SortByLabel, sortAttributes, 551, LABEL_MASKS("%T","%R"));  // Title, Rating | empty, empty

        const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavepisodes");
        SetSortMethod(viewState->m_sortDescription);
        SetViewAsControl(viewState->m_viewMode);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
        break;
      }
      case NodeType::RECENTLY_ADDED_EPISODES:
      {
        AddSortMethod(SortByNone, 552, LABEL_MASKS("%Z - %H. %T", "%R"));  // TvShow - Order. Title, Rating | empty, empty
        SetSortMethod(SortByNone);

        SetViewAsControl(CViewStateSettings::GetInstance().Get("videonavepisodes")->m_viewMode);
        SetSortOrder(SortOrderNone);

        break;
      }
      case NodeType::TITLE_MOVIES:
      {
        if (params.GetSetId() > -1) // Is this a listing within a set?
        {
          AddSortMethod(SortByYear, 562, LABEL_MASKS("%T", "%Y"));  // Title, Year | empty, empty
          AddSortMethod(SortBySortTitle, sortAttributes, 556, LABEL_MASKS("%T", "%R"));  // Title, Rating | empty, empty
          AddSortMethod(SortByOriginalTitle, sortAttributes, 20376,
                        LABEL_MASKS("%T", "%R")); // Title, Rating | empty, empty
        }
        else
        {
          AddSortMethod(SortBySortTitle, sortAttributes, 556, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
          AddSortMethod(SortByOriginalTitle, sortAttributes, 20376,
                        LABEL_MASKS("%T", "%R", "%T", "%R")); // Title, Rating | Title, Rating
          AddSortMethod(SortByYear, 562, LABEL_MASKS("%T", "%Y", "%T", "%Y"));  // Title, Year | Title, Year
        }
        AddSortMethod(SortByRating, 563, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
        AddSortMethod(SortByUserRating, 38018, LABEL_MASKS("%T", "%r", "%T", "%r"));  // Title, Userrating | Title, Userrating
        AddSortMethod(SortByMPAA, 20074, LABEL_MASKS("%T", "%O"));  // Title, MPAA | empty, empty
        AddSortMethod(SortByTime, 180, LABEL_MASKS("%T", "%D"));  // Title, Duration | empty, empty
        AddSortMethod(SortByDateAdded, 570, LABEL_MASKS("%T", "%a", "%T", "%a"));  // Title, DateAdded | Title, DateAdded
        AddSortMethod(SortByLastPlayed, 568, LABEL_MASKS("%T", "%p", "%T", "%p"));  // Title, #Last played | Title, #Last played
        AddSortMethod(SortByPlaycount, 567,
                      LABEL_MASKS("%T", "%V", "%T", "%V")); // Title, Playcount | Title, Playcount

        const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavtitles");
        if (params.GetSetId() > -1)
        {
          SetSortMethod(SortByYear);
          SetSortOrder(SortOrderAscending);
        }
        else
        {
          SetSortMethod(viewState->m_sortDescription);
          SetSortOrder(viewState->m_sortDescription.sortOrder);
        }

        SetViewAsControl(viewState->m_viewMode);
      }
      break;
      case NodeType::TITLE_MUSICVIDEOS:
      {
        AddSortMethod(SortByLabel, sortAttributes, 551, LABEL_MASKS("%T - %A", "%Y"));  // Title, Artist, Year | empty, empty
        AddSortMethod(SortByAlbum, sortAttributes, 558, LABEL_MASKS("%B - %T - %A", "%Y"));  // Album, Title, Artist, Year | empty, empty
        AddSortMethod(SortByArtist, sortAttributes, 557, LABEL_MASKS("%A - %T", "%Y"));  // Artist - Title, Year | empty, empty
        AddSortMethod(SortByArtistThenYear, sortAttributes, 578, LABEL_MASKS("%A - %T", "%Y"));  // Artist, Title, Year| empty, empty
        AddSortMethod(SortByYear, 562, LABEL_MASKS("%T - %A", "%Y")); // Title, Artist, Year| empty, empty
        AddSortMethod(SortByTime, 180, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
        AddSortMethod(SortByDateAdded, 570, LABEL_MASKS("%T - %A", "%a"));  // Title - Artist, DateAdded | empty, empty
        AddSortMethod(SortByPlaycount, 567, LABEL_MASKS("%T - %A", "%V"));  // Title - Artist, PlayCount
        AddSortMethod(SortByMPAA, 20074, LABEL_MASKS("%T - %A", "%O")); // Title - Artist, MPAARating
        AddSortMethod(SortByUserRating, 38018, LABEL_MASKS("%T - %A", "%r"));  // Title - Artist, UserRating

        std::string strTrack=CServiceBroker::GetSettingsComponent()->GetSettings()->GetString(CSettings::SETTING_MUSICFILES_TRACKFORMAT);
        AddSortMethod(SortByTrackNumber, 554, LABEL_MASKS(strTrack, "%N"));  // Userdefined, Track Number | empty, empty

        const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavmusicvideos");
        SetSortMethod(viewState->m_sortDescription);
        SetViewAsControl(viewState->m_viewMode);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
      }
      break;
      case NodeType::RECENTLY_ADDED_MOVIES:
      {
        AddSortMethod(SortByNone, 552, LABEL_MASKS("%T", "%R"));  // Title, Rating | empty, empty
        SetSortMethod(SortByNone);

        SetViewAsControl(CViewStateSettings::GetInstance().Get("videonavtitles")->m_viewMode);

        SetSortOrder(SortOrderNone);
      }
      break;
      case NodeType::RECENTLY_ADDED_MUSICVIDEOS:
      {
        AddSortMethod(SortByNone, 552, LABEL_MASKS("%A - %T", "%Y"));  // Artist - Title, Year | empty, empty
        SetSortMethod(SortByNone);

        SetViewAsControl(CViewStateSettings::GetInstance().Get("videonavmusicvideos")->m_viewMode);

        SetSortOrder(SortOrderNone);
      }
      break;
      case NodeType::MOVIE_ASSETS:
      case NodeType::MOVIE_ASSETS_VERSIONS:
      {
        AddSortMethod(SortByLabel, sortAttributes, 551,
                      LABEL_MASKS("%L", "", "%L", "")); // Label, empty | Label, empty
        AddSortMethod(SortByTime, 180,
                      LABEL_MASKS("%L", "%D", "%L", "")); // Label, Duration | Label, empty
        AddSortMethod(SortByVideoResolution, 21443,
                      LABEL_MASKS("%L", "", "%L", "")); // Label, empty | Label, empty
        AddSortMethod(SortByVideoCodec, 21445,
                      LABEL_MASKS("%L", "", "%L", "")); // Label, empty | Label, empty
        AddSortMethod(SortByAudioCodec, 21446,
                      LABEL_MASKS("%L", "", "%L", "")); // Label, empty | Label, empty
        AddSortMethod(SortByDateAdded, 570,
                      LABEL_MASKS("%L", "%a", "%L", "")); // Label, DateAdded | Label, empty
        AddSortMethod(SortByLastPlayed, SortAttributeForceConsiderFolders, 568,
                      LABEL_MASKS("%L", "%p", "%L", "")); // Label, #Last played | Label, empty
        AddSortMethod(SortByPlaycount, SortAttributeForceConsiderFolders, 567,
                      LABEL_MASKS("%L", "%V", "%L", "")); // Label, Playcount | Label, empty

        const CViewState* viewState = CViewStateSettings::GetInstance().Get(
            nodeType == NodeType::MOVIE_ASSETS ? "videonavassets" : "videonavversions");
        SetSortMethod(viewState->m_sortDescription);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
        SetViewAsControl(viewState->m_viewMode);
      }
      break;
      case NodeType::MOVIE_ASSETS_EXTRAS:
      {
        AddSortMethod(SortByLabel, sortAttributes, 551,
                      LABEL_MASKS("%L", "", "%L", "")); // Label, empty | Label, empty
        AddSortMethod(SortByTime, 180,
                      LABEL_MASKS("%L", "%D", "%L", "")); // Label, Duration | Label, empty
        AddSortMethod(SortByDateAdded, 570,
                      LABEL_MASKS("%L", "%a", "%L", "")); // Label, DateAdded | Label, empty
        AddSortMethod(SortByLastPlayed, SortAttributeForceConsiderFolders, 568,
                      LABEL_MASKS("%L", "%p", "%L", "")); // Label, #Last played | Label, empty
        AddSortMethod(SortByPlaycount, SortAttributeForceConsiderFolders, 567,
                      LABEL_MASKS("%L", "%V", "%L", "")); // Label, Playcount | Label, empty

        const CViewState* viewState = CViewStateSettings::GetInstance().Get("videonavextras");
        SetSortMethod(viewState->m_sortDescription);
        SetSortOrder(viewState->m_sortDescription.sortOrder);
        SetViewAsControl(viewState->m_viewMode);
      }
      break;

      default:
        break;
    }
  }
  else
  {
    AddSortMethod(SortByLabel, sortAttributes, 551, LABEL_MASKS("%L", "%I", "%L", ""));  // Label, Size | Label, empty
    AddSortMethod(SortBySize, 553, LABEL_MASKS("%L", "%I", "%L", "%I"));  // Label, Size | Label, Size
    AddSortMethod(SortByDate, 552, LABEL_MASKS("%L", "%J", "%L", "%J"));  // Label, Date | Label, Date
    AddSortMethod(SortByFile, 561, LABEL_MASKS("%L", "%I", "%L", ""));  // Label, Size | Label, empty

    const CViewState *viewState = CViewStateSettings::GetInstance().Get("videofiles");
    SetSortMethod(viewState->m_sortDescription);
    SetViewAsControl(viewState->m_viewMode);
    SetSortOrder(viewState->m_sortDescription.sortOrder);
  }
  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateWindowVideoNav::SaveViewState()
{
  if (VIDEO::IsVideoDb(m_items))
  {
    NodeType nodeType = CVideoDatabaseDirectory::GetDirectoryChildType(m_items.GetPath());
    CQueryParams params;
    CVideoDatabaseDirectory::GetQueryParams(m_items.GetPath(),params);
    switch (nodeType)
    {
      case NodeType::ACTOR:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     CViewStateSettings::GetInstance().Get("videonavactors"));
        break;
      case NodeType::YEAR:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     CViewStateSettings::GetInstance().Get("videonavyears"));
        break;
      case NodeType::GENRE:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     CViewStateSettings::GetInstance().Get("videonavgenres"));
        break;
      case NodeType::TITLE_MOVIES:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     params.GetSetId() > -1
                         ? NULL
                         : CViewStateSettings::GetInstance().Get("videonavtitles"));
        break;
      case NodeType::EPISODES:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     CViewStateSettings::GetInstance().Get("videonavepisodes"));
        break;
      case NodeType::TITLE_TVSHOWS:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     CViewStateSettings::GetInstance().Get("videonavtvshows"));
        break;
      case NodeType::SEASONS:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     CViewStateSettings::GetInstance().Get("videonavseasons"));
        break;
      case NodeType::TITLE_MUSICVIDEOS:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     CViewStateSettings::GetInstance().Get("videonavmusicvideos"));
        break;
      case NodeType::MOVIE_ASSETS:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     CViewStateSettings::GetInstance().Get("videonavassets"));
        break;
      case NodeType::MOVIE_ASSETS_VERSIONS:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     CViewStateSettings::GetInstance().Get("videonavversions"));
        break;
      case NodeType::MOVIE_ASSETS_EXTRAS:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV,
                     CViewStateSettings::GetInstance().Get("videonavextras"));
        break;
      default:
        SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV);
        break;
    }
  }
  else
  {
    SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, CViewStateSettings::GetInstance().Get("videofiles"));
  }
}

std::vector<CMediaSource>& CGUIViewStateWindowVideoNav::GetSources()
{
  //  Setup shares we want to have
  m_sources.clear();
  CFileItemList items;
  if (CServiceBroker::GetSettingsComponent()->GetSettings()->GetBool(CSettings::SETTING_MYVIDEOS_FLATTEN))
    CDirectory::GetDirectory("library://video_flat/", items, "", DIR_FLAG_DEFAULTS);
  else
    CDirectory::GetDirectory("library://video/", items, "", DIR_FLAG_DEFAULTS);
  for (int i=0; i<items.Size(); ++i)
  {
    CFileItemPtr item=items[i];
    CMediaSource share;
    share.strName=item->GetLabel();
    share.strPath = item->GetPath();
    share.m_strThumbnailImage = item->GetArt("icon");
    share.m_iDriveType = SourceType::LOCAL;
    m_sources.push_back(share);
  }
  return CGUIViewStateWindowVideo::GetSources();
}

bool CGUIViewStateWindowVideoNav::AutoPlayNextItem()
{
  CQueryParams params;
  CVideoDatabaseDirectory::GetQueryParams(m_items.GetPath(),params);
  if (static_cast<VideoDbContentType>(params.GetContentType()) == VideoDbContentType::MUSICVIDEOS)
    return CServiceBroker::GetSettingsComponent()->GetSettings()->GetBool(CSettings::SETTING_MUSICPLAYER_AUTOPLAYNEXTITEM);

  return CGUIViewStateWindowVideo::AutoPlayNextItem();
}

CGUIViewStateWindowVideoPlaylist::CGUIViewStateWindowVideoPlaylist(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  AddSortMethod(SortByNone, 551, LABEL_MASKS("%L", "", "%L", ""));  // Label, empty | Label, empty
  SetSortMethod(SortByNone);

  SetViewAsControl(DEFAULT_VIEW_LIST);

  SetSortOrder(SortOrderNone);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_PLAYLIST);
}

void CGUIViewStateWindowVideoPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_PLAYLIST);
}

bool CGUIViewStateWindowVideoPlaylist::HideExtensions()
{
  return true;
}

bool CGUIViewStateWindowVideoPlaylist::HideParentDirItems()
{
  return true;
}

std::vector<CMediaSource>& CGUIViewStateWindowVideoPlaylist::GetSources()
{
  m_sources.clear();
  //  Playlist share
  CMediaSource share;
  share.strPath= "playlistvideo://";
  share.m_iDriveType = SourceType::LOCAL;
  m_sources.push_back(share);

  // no plugins in playlist window
  return m_sources;
}

CGUIViewStateVideoMovies::CGUIViewStateVideoMovies(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  SortAttribute sortAttributes = SortAttributeNone;
  if (CServiceBroker::GetSettingsComponent()->GetSettings()->GetBool(
          CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING))
    sortAttributes = SortAttributeIgnoreArticle;

  AddSortMethod(SortBySortTitle, sortAttributes, 556,
                LABEL_MASKS("%T", "%R", "%T", "%R")); // Title, Rating | Title, Rating
  AddSortMethod(SortByOriginalTitle, sortAttributes, 20376,
                LABEL_MASKS("%T", "%R", "%T", "%R")); // Title, Rating | Title, Rating
  AddSortMethod(SortByYear, 562, LABEL_MASKS("%T", "%Y", "%T", "%Y"));  // Title, Year | Title, Year
  AddSortMethod(SortByRating, 563, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
  AddSortMethod(SortByUserRating, 38018, LABEL_MASKS("%T", "%r", "%T", "%r"));  // Title, Userrating | Title, Userrating
  AddSortMethod(SortByMPAA, 20074, LABEL_MASKS("%T", "%O"));  // Title, MPAA | empty, empty
  AddSortMethod(SortByTime, 180, LABEL_MASKS("%T", "%D"));  // Title, Duration | empty, empty
  AddSortMethod(SortByDateAdded, 570, LABEL_MASKS("%T", "%a", "%T", "%a"));  // Title, DateAdded | Title, DateAdded
  AddSortMethod(SortByPlaycount, 567,
                LABEL_MASKS("%T", "%V", "%T", "%V")); // Title, Playcount | Title, Playcount

  const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavtitles");
  if (PLAYLIST::IsSmartPlayList(items) || items.IsLibraryFolder())
    AddPlaylistOrder(items, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
  else
  {
    SetSortMethod(viewState->m_sortDescription);
    SetSortOrder(viewState->m_sortDescription.sortOrder);
  }

  SetViewAsControl(viewState->m_viewMode);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateVideoMovies::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, CViewStateSettings::GetInstance().Get("videonavtitles"));
}

CGUIViewStateVideoMusicVideos::CGUIViewStateVideoMusicVideos(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  SortAttribute sortAttributes = SortAttributeNone;
  const std::shared_ptr<CSettings> settings = CServiceBroker::GetSettingsComponent()->GetSettings();
  if (settings->GetBool(CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING))
    sortAttributes = SortAttributeIgnoreArticle;

  AddSortMethod(SortByLabel, sortAttributes, 551, LABEL_MASKS("%T - %A", "%Y"));  // Title, Artist, Year | empty, empty
  AddSortMethod(SortByAlbum, sortAttributes, 558, LABEL_MASKS("%B - %T - %A", "%Y"));  // Album, Title, Artist, Year | empty, empty
  AddSortMethod(SortByArtist, sortAttributes, 557, LABEL_MASKS("%A - %T", "%Y"));  // Artist - Title, Year | empty, empty
  AddSortMethod(SortByArtistThenYear, sortAttributes, 578, LABEL_MASKS("%A - %T", "%Y"));  // Artist, Title, Year| empty, empty
  AddSortMethod(SortByYear, 562, LABEL_MASKS("%T - %A", "%Y")); // Title, Artist, Year| empty, empty
  AddSortMethod(SortByTime, 180, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
  AddSortMethod(SortByDateAdded, 570, LABEL_MASKS("%T - %A", "%a"));  // Title - Artist, DateAdded | empty, empty
  AddSortMethod(SortByPlaycount, 567, LABEL_MASKS("%T - %A", "%V"));  // Title - Artist, PlayCount
  AddSortMethod(SortByMPAA, 20074, LABEL_MASKS("%T - %A", "%O")); // Title - Artist, MPAARating
  AddSortMethod(SortByUserRating, 38018, LABEL_MASKS("%T - %A", "%r"));  // Title - Artist, UserRating

  std::string strTrack = settings->GetString(CSettings::SETTING_MUSICFILES_TRACKFORMAT);
  AddSortMethod(SortByTrackNumber, 554, LABEL_MASKS(strTrack, "%N"));  // Userdefined, Track Number | empty, empty

  const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavmusicvideos");
  if (PLAYLIST::IsSmartPlayList(items) || items.IsLibraryFolder())
    AddPlaylistOrder(items, LABEL_MASKS("%A - %T", "%Y"));  // Artist - Title, Year | empty, empty
  else
  {
    SetSortMethod(viewState->m_sortDescription);
    SetSortOrder(viewState->m_sortDescription.sortOrder);
  }

  SetViewAsControl(viewState->m_viewMode);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateVideoMusicVideos::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, CViewStateSettings::GetInstance().Get("videonavmusicvideos"));
}

CGUIViewStateVideoTVShows::CGUIViewStateVideoTVShows(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  SortAttribute sortAttributes = SortAttributeNone;
  const std::shared_ptr<CSettings> settings = CServiceBroker::GetSettingsComponent()->GetSettings();
  if (settings->GetBool(CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING))
    sortAttributes = SortAttributeIgnoreArticle;

  AddSortMethod(SortBySortTitle, sortAttributes, 556,
                LABEL_MASKS("%T", "%M", "%T", "%M")); // Title, #Episodes | Title, #Episodes
  AddSortMethod(SortByNumberOfEpisodes, sortAttributes, 20360,
                LABEL_MASKS("%L", "%M", "%L", "%M")); // Label, #Episodes | Label, #Episodes
  AddSortMethod(SortByLastPlayed, sortAttributes, 568,
                LABEL_MASKS("%T", "%p", "%T", "%p")); // Title, #Last played | Title, #Last played
  AddSortMethod(SortByDateAdded, sortAttributes, 570,
                LABEL_MASKS("%T", "%a", "%T", "%a")); // Title, DateAdded | Title, DateAdded
  AddSortMethod(SortByYear, sortAttributes, 562,
                LABEL_MASKS("%T", "%Y", "%T", "%Y")); // Title, Year | Title, Year
  AddSortMethod(SortByUserRating, sortAttributes, 38018,
                LABEL_MASKS("%T", "%r", "%T", "%r")); // Title, Userrating | Title, Userrating

  const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavtvshows");
  if (PLAYLIST::IsSmartPlayList(items) || items.IsLibraryFolder())
    AddPlaylistOrder(items, LABEL_MASKS("%T", "%M", "%T", "%M"));  // Title, #Episodes | Title, #Episodes
  else
  {
    SetSortMethod(viewState->m_sortDescription);
    SetSortOrder(viewState->m_sortDescription.sortOrder);
  }

  SetViewAsControl(viewState->m_viewMode);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateVideoTVShows::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, CViewStateSettings::GetInstance().Get("videonavtvshows"));
}

CGUIViewStateVideoEpisodes::CGUIViewStateVideoEpisodes(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  // TvShow - Order. Title, Rating | empty, empty
  AddSortMethod(SortByEpisodeNumber, 20359, LABEL_MASKS("%Z - %H. %T","%R"));
  // TvShow - Order. Title, Rating | empty, empty
  AddSortMethod(SortByRating, 563, LABEL_MASKS("%Z - %H. %T", "%R"));
  // TvShow - Order. Title, Userrating | empty, empty
  AddSortMethod(SortByUserRating, 38018, LABEL_MASKS("%Z - %H. %T", "%r"));
  // TvShow - Order. Title, MPAA | empty, empty
  AddSortMethod(SortByMPAA, 20074, LABEL_MASKS("%Z - %H. %T", "%O"));
  // TvShow - Order. Title, Production Code | empty, empty
  AddSortMethod(SortByProductionCode, 20368, LABEL_MASKS("%Z - %H. %T","%P"));
  // TvShow - Order. Title, Date | empty, empty
  AddSortMethod(SortByDate, 552, LABEL_MASKS("%Z - %H. %T","%J"));
  // TvShow - Order. Title, Playcount | empty, empty
  AddSortMethod(SortByPlaycount, 567, LABEL_MASKS("%H. %T", "%V"));

  AddSortMethod(SortByLabel, 551, LABEL_MASKS("%Z - %H. %T","%R"),  // TvShow - Order. Title, Rating | empty, empty
    CServiceBroker::GetSettingsComponent()->GetSettings()->GetBool(CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING) ? SortAttributeIgnoreArticle : SortAttributeNone);

  const CViewState *viewState = CViewStateSettings::GetInstance().Get("videonavepisodes");
  if (PLAYLIST::IsSmartPlayList(items) || items.IsLibraryFolder())
    AddPlaylistOrder(items, LABEL_MASKS("%Z - %H. %T", "%R"));  // TvShow - Order. Title, Rating | empty, empty
  else
  {
    SetSortMethod(viewState->m_sortDescription);
    SetSortOrder(viewState->m_sortDescription.sortOrder);
  }

  SetViewAsControl(viewState->m_viewMode);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateVideoEpisodes::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, CViewStateSettings::GetInstance().Get("videonavepisodes"));
}

CGUIViewStateVideoPlaylist::CGUIViewStateVideoPlaylist(const CFileItemList& items)
  : CGUIViewStateWindowVideo(items)
{
  SortAttribute sortAttributes = SortAttributeNone;
  if (CServiceBroker::GetSettingsComponent()->GetSettings()->GetBool(
          CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING))
    sortAttributes = SortAttributeIgnoreArticle;

  AddSortMethod(SortByPlaylistOrder, 559, LABEL_MASKS("%L", "")); // Label, empty
  AddSortMethod(SortByLabel, sortAttributes, 551,
                LABEL_MASKS("%L", "%I", "%L", "")); // Label, Size | Label, empty
  AddSortMethod(SortBySize, 553, LABEL_MASKS("%L", "%I", "%L", "%I")); // Label, Size | Label, Size
  AddSortMethod(SortByDate, 552, LABEL_MASKS("%L", "%J", "%L", "%J")); // Label, Date | Label, Date
  AddSortMethod(SortByFile, 561, LABEL_MASKS("%L", "%I", "%L", "")); // Label, Size | Label, empty

  SetSortMethod(SortByPlaylistOrder);

  const CViewState* viewState = CViewStateSettings::GetInstance().Get("videofiles");
  SetViewAsControl(viewState->m_viewMode);
  SetSortOrder(viewState->m_sortDescription.sortOrder);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateVideoPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV);
}
