/* Configurations.c- misc. configurations
 *
 *  WPrefs - Window Maker Preferences Program
 *
 *  Copyright (c) 1998-2003 Alfredo K. Kojima
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 *  USA.
 */

#include "WPrefs.h"

typedef struct _Panel {
	WMBox *box;
	char *sectionName;

	char *description;

	CallbackRec callbacks;

	WMWidget *parent;

	WMFrame *icoF;
	WMButton *icoB[5];

	WMFrame *shaF;
	WMButton *shaB[5];

	WMFrame *titlF;
	WMButton *oldsB;
	WMButton *newsB;
	WMButton *nextB;

	WMFrame *animF;
	WMButton *animB;
	WMButton *supB;
	WMLabel *noteL;

	WMFrame *smoF;
	WMButton *smoB;

	WMFrame *dithF;
	WMButton *dithB;
	WMSlider *dithS;
	WMLabel *dithL;
	WMLabel *dith1L;
	WMLabel *dith2L;

	int cmapSize;
} _Panel;

#define ICON_FILE	"configs"
#define OLDS_IMAGE	"oldstyle"
#define NEWS_IMAGE	"newstyle"
#define NEXT_IMAGE	"nextstyle"
#define ANIM_IMAGE	"animations"
#define SUPERF_IMAGE	"moreanim"
#define SMOOTH_IMAGE	"smooth"
#define SPEED_IMAGE 	"speed%i"
#define SPEED_IMAGE_S 	"speed%is"
#define ARQUIVO_XIS	"xis"

static void updateLabel(WMWidget *self, void *data);

static void showData(_Panel *panel)
{
	char *str;

	WMPerformButtonClick(panel->icoB[GetSpeedForKey("IconSlideSpeed")]);
	WMPerformButtonClick(panel->shaB[GetSpeedForKey("ShadeSpeed")]);

	str = GetStringForKey("Newstyle");
	if (str && strcasecmp(str, "next") == 0) {
		WMPerformButtonClick(panel->nextB);
	} else if (str && strcasecmp(str, "old") == 0) {
		WMPerformButtonClick(panel->oldsB);
	} else {
		WMPerformButtonClick(panel->newsB);
	}

	WMSetButtonSelected(panel->animB, !GetBoolForKey("DisableAnimations"));
	WMSetButtonSelected(panel->supB, GetBoolForKey("Superfluous"));
	WMSetButtonSelected(panel->smoB, GetBoolForKey("SmoothWorkspaceBack"));
	WMSetButtonSelected(panel->dithB, GetBoolForKey("DisableDithering"));
	WMSetSliderValue(panel->dithS, GetIntegerForKey("ColormapSize"));
	updateLabel(panel->dithS, panel);
}

static void updateLabel(WMWidget *self, void *data)
{
	WMSlider *sPtr = (WMSlider *) self;
	_Panel *panel = (_Panel *) data;
	char buffer[64];
	float fl;

	fl = WMGetSliderValue(sPtr);
	panel->cmapSize = (int)fl;

	sprintf(buffer, "%i", panel->cmapSize * panel->cmapSize * panel->cmapSize);
	WMSetLabelText(panel->dithL, buffer);
}

static void
createImages(WMScreen *scr, RContext *rc, RImage *xis, char *file,
	     WMPixmap **icon1, WMPixmap **icon2)
{
	RImage *icon;
	char *path;
	RColor gray = { 0xae, 0xaa, 0xae, 0 };

	*icon1 = NULL;
	*icon2 = NULL;

	path = LocateImage(file);
	if (!path)
		return;

	*icon1 = WMCreatePixmapFromFile(scr, path);
	if (!*icon1) {
		wwarning(_("could not load icon %s"), path);
		wfree(path);
		return;
	}
	icon = RLoadImage(rc, path, 0);
	if (!icon) {
		wwarning(_("could not load icon %s"), path);
		wfree(path);
		return;
	}
	RCombineImageWithColor(icon, &gray);
	if (xis) {
		RCombineImagesWithOpaqueness(icon, xis, 180);
		if (!(*icon2 = WMCreatePixmapFromRImage(scr, icon, 127)))
			wwarning(_("could not process icon %s: %s"), file, RMessageForError(RErrorCode));
	}
	RReleaseImage(icon);
	wfree(path);
}

static void createPanel(Panel *p)
{
	_Panel *panel = (_Panel *) p;
	WMScreen *scr = WMWidgetScreen(panel->parent);
	char *buf1, *buf2;
	WMPixmap *icon, *altIcon;
	RImage *xis = NULL;
	int i;
	RContext *rc = WMScreenRContext(scr);
	WMFont *font = WMSystemFontOfSize(scr, 10);
	char *path;

	path = LocateImage(ARQUIVO_XIS);
	if (path) {
		xis = RLoadImage(rc, path, 0);
		if (!xis)
			wwarning(_("could not load image file %s"), path);
		wfree(path);
	}

	panel->box = WMCreateBox(panel->parent);
	WMSetViewExpandsToParent(WMWidgetView(panel->box), 2, 2, 2, 2);

    /*********** Icon Slide Speed **********/
	panel->icoF = WMCreateFrame(panel->box);
	WMResizeWidget(panel->icoF, 230, 45);
	WMMoveWidget(panel->icoF, 15, 10);
	WMSetFrameTitle(panel->icoF, _("Icon Slide Speed"));

    /*********** Shade Animation Speed **********/
	panel->shaF = WMCreateFrame(panel->box);
	WMResizeWidget(panel->shaF, 230, 45);
	WMMoveWidget(panel->shaF, 15, 60);
	WMSetFrameTitle(panel->shaF, _("Shade Animation Speed"));

	buf1 = wmalloc(strlen(SPEED_IMAGE) + 1);
	buf2 = wmalloc(strlen(SPEED_IMAGE_S) + 1);

	for (i = 0; i < 5; i++) {
		panel->icoB[i] = WMCreateCustomButton(panel->icoF, WBBStateChangeMask);
		panel->shaB[i] = WMCreateCustomButton(panel->shaF, WBBStateChangeMask);
		WMResizeWidget(panel->icoB[i], 40, 24);
		WMMoveWidget(panel->icoB[i], 10 + (40 * i), 15);
		WMResizeWidget(panel->shaB[i], 40, 24);
		WMMoveWidget(panel->shaB[i], 10 + (40 * i), 15);
		WMSetButtonBordered(panel->icoB[i], False);
		WMSetButtonImagePosition(panel->icoB[i], WIPImageOnly);
		if (i > 0) {
			WMGroupButtons(panel->icoB[0], panel->icoB[i]);
		}
		WMSetButtonBordered(panel->shaB[i], False);
		WMSetButtonImagePosition(panel->shaB[i], WIPImageOnly);
		if (i > 0) {
			WMGroupButtons(panel->shaB[0], panel->shaB[i]);
		}
		sprintf(buf1, SPEED_IMAGE, i);
		sprintf(buf2, SPEED_IMAGE_S, i);
		path = LocateImage(buf1);
		if (path) {
			icon = WMCreatePixmapFromFile(scr, path);
			if (icon) {
				WMSetButtonImage(panel->icoB[i], icon);
				WMSetButtonImage(panel->shaB[i], icon);
				WMReleasePixmap(icon);
			} else {
				wwarning(_("could not load icon file %s"), path);
			}
			wfree(path);
		}
		path = LocateImage(buf2);
		if (path) {
			icon = WMCreatePixmapFromFile(scr, path);
			if (icon) {
				WMSetButtonAltImage(panel->icoB[i], icon);
				WMSetButtonAltImage(panel->shaB[i], icon);
				WMReleasePixmap(icon);
			} else {
				wwarning(_("could not load icon file %s"), path);
			}
			wfree(path);
		}
	}
	wfree(buf1);
	wfree(buf2);

	WMMapSubwidgets(panel->icoF);
	WMMapSubwidgets(panel->shaF);

    /***************** Smoothed Scaling *****************/
	panel->smoF = WMCreateFrame(panel->box);
	WMResizeWidget(panel->smoF, 115, 110);
	WMMoveWidget(panel->smoF, 18, 115);
	WMSetFrameTitle(panel->smoF, _("Smooth Scaling"));
	WMSetBalloonTextForView(_("Smooth scaled background images, neutralizing\n"
				  "the `pixelization' effect. This will slow\n"
				  "down loading of background images considerably."), WMWidgetView(panel->smoF));

	panel->smoB = WMCreateButton(panel->smoF, WBTToggle);
	WMResizeWidget(panel->smoB, 64, 64);
	WMMoveWidget(panel->smoB, 25, 25);
	WMSetButtonImagePosition(panel->smoB, WIPImageOnly);
	path = LocateImage(SMOOTH_IMAGE);
	if (path) {
		RImage *image, *scaled;

		image = RLoadImage(WMScreenRContext(scr), path, 0);
		wfree(path);

		scaled = RScaleImage(image, 61, 61);
		icon = WMCreatePixmapFromRImage(scr, scaled, 128);
		RReleaseImage(scaled);
		if (icon) {
			WMSetButtonImage(panel->smoB, icon);
			WMReleasePixmap(icon);
		}

		scaled = RSmoothScaleImage(image, 61, 61);
		icon = WMCreatePixmapFromRImage(scr, scaled, 128);
		RReleaseImage(scaled);
		if (icon) {
			WMSetButtonAltImage(panel->smoB, icon);
			WMReleasePixmap(icon);
		}

		RReleaseImage(image);
	}

	WMMapSubwidgets(panel->smoF);

    /***************** Titlebar Style Size ****************/
	panel->titlF = WMCreateFrame(panel->box);
	WMResizeWidget(panel->titlF, 105, 110);
	WMMoveWidget(panel->titlF, 140, 115);
	WMSetFrameTitle(panel->titlF, _("Titlebar Style"));

	panel->newsB = WMCreateButton(panel->titlF, WBTOnOff);
	WMResizeWidget(panel->newsB, 74, 40);
	WMMoveWidget(panel->newsB, 15, 20);
	WMSetButtonImagePosition(panel->newsB, WIPImageOnly);
	path = LocateImage(NEWS_IMAGE);
	if (path) {
		icon = WMCreatePixmapFromFile(scr, path);
		if (icon) {
			WMSetButtonImage(panel->newsB, icon);
			WMReleasePixmap(icon);
		}
		wfree(path);
	}

	panel->oldsB = WMCreateButton(panel->titlF, WBTOnOff);
    WMResizeWidget(panel->oldsB, 37, 40);
	WMMoveWidget(panel->oldsB, 15, 60);
	WMSetButtonImagePosition(panel->oldsB, WIPImageOnly);
	path = LocateImage(OLDS_IMAGE);
	if (path) {
		icon = WMCreatePixmapFromFile(scr, path);
		if (icon) {
			WMSetButtonImage(panel->oldsB, icon);
			WMReleasePixmap(icon);
		}
		wfree(path);
	}

	panel->nextB = WMCreateButton(panel->titlF, WBTOnOff);
	WMResizeWidget(panel->nextB, 37, 40);
	WMMoveWidget(panel->nextB, 52, 60);
	WMSetButtonImagePosition(panel->nextB, WIPImageOnly);
	path = LocateImage(NEXT_IMAGE);
	if (path) {
		icon = WMCreatePixmapFromFile(scr, path);
		if (icon) {
			WMSetButtonImage(panel->nextB, icon);
			WMReleasePixmap(icon);
		}
		wfree(path);
	}

	WMGroupButtons(panel->newsB, panel->oldsB);
	WMGroupButtons(panel->newsB, panel->nextB);

	WMMapSubwidgets(panel->titlF);

    /**************** Features ******************/
	panel->animF = WMCreateFrame(panel->box);
	WMResizeWidget(panel->animF, 255, 115);
	WMMoveWidget(panel->animF, 255, 10);
	WMSetFrameTitle(panel->animF, _("Animations"));

	panel->animB = WMCreateButton(panel->animF, WBTToggle);
	WMResizeWidget(panel->animB, 64, 64);
	WMMoveWidget(panel->animB, 15, 20);
	WMSetButtonFont(panel->animB, font);
	WMSetButtonText(panel->animB, _("Animations"));
	WMSetButtonImagePosition(panel->animB, WIPAbove);
	createImages(scr, rc, xis, ANIM_IMAGE, &altIcon, &icon);
	if (icon) {
		WMSetButtonImage(panel->animB, icon);
		WMReleasePixmap(icon);
	}
	if (altIcon) {
		WMSetButtonAltImage(panel->animB, altIcon);
		WMReleasePixmap(altIcon);
	}
	WMSetBalloonTextForView(_("Disable/enable animations such as those shown\n"
				  "for window miniaturization, shading etc."), WMWidgetView(panel->animB));

	panel->supB = WMCreateButton(panel->animF, WBTToggle);
	WMResizeWidget(panel->supB, 64, 64);
	WMMoveWidget(panel->supB, 95, 20);
	WMSetButtonFont(panel->supB, font);
	WMSetButtonText(panel->supB, _("Superfluous"));
	WMSetButtonImagePosition(panel->supB, WIPAbove);
	createImages(scr, rc, xis, SUPERF_IMAGE, &altIcon, &icon);
	if (icon) {
		WMSetButtonImage(panel->supB, icon);
		WMReleasePixmap(icon);
	}
	if (altIcon) {
		WMSetButtonAltImage(panel->supB, altIcon);
		WMReleasePixmap(altIcon);
	}
	WMSetBalloonTextForView(_("Disable/enable `superfluous' features and\n"
				  "animations. These include the `ghosting' of the\n"
				  "dock when it's being moved to another side and\n"
				  "the explosion animation when undocking icons."), WMWidgetView(panel->supB));

	WMMapSubwidgets(panel->animF);

    /*********** Dithering **********/
	panel->cmapSize = 4;

	panel->dithF = WMCreateFrame(panel->box);
	WMResizeWidget(panel->dithF, 255, 95);
	WMMoveWidget(panel->dithF, 255, 130);
	WMSetFrameTitle(panel->dithF, _("Dithering colormap for 8bpp"));

	WMSetBalloonTextForView(_("Number of colors to reserve for Window Maker\n"
				  "on displays that support only 8bpp (PseudoColor)."),
				WMWidgetView(panel->dithF));

	panel->dithB = WMCreateSwitchButton(panel->dithF);
	WMResizeWidget(panel->dithB, 235, 32);
	WMMoveWidget(panel->dithB, 15, 15);
	WMSetButtonText(panel->dithB, _("Disable dithering in any visual/depth"));

	panel->dithL = WMCreateLabel(panel->dithF);
	WMResizeWidget(panel->dithL, 75, 16);
	WMMoveWidget(panel->dithL, 90, 50);
	WMSetLabelTextAlignment(panel->dithL, WACenter);
	WMSetLabelText(panel->dithL, "64");

	panel->dithS = WMCreateSlider(panel->dithF);
	WMResizeWidget(panel->dithS, 95, 16);
	WMMoveWidget(panel->dithS, 80, 65);
	WMSetSliderMinValue(panel->dithS, 2);
	WMSetSliderMaxValue(panel->dithS, 6);
	WMSetSliderContinuous(panel->dithS, True);
	WMSetSliderAction(panel->dithS, updateLabel, panel);

	panel->dith1L = WMCreateLabel(panel->dithF);
	WMResizeWidget(panel->dith1L, 70, 35);
	WMMoveWidget(panel->dith1L, 5, 50);
	WMSetLabelTextAlignment(panel->dith1L, WACenter);
	WMSetLabelFont(panel->dith1L, font);
	WMSetLabelText(panel->dith1L, _("More colors for\napplications"));

	panel->dith2L = WMCreateLabel(panel->dithF);
	WMResizeWidget(panel->dith2L, 70, 35);
	WMMoveWidget(panel->dith2L, 180, 50);
	WMSetLabelTextAlignment(panel->dith2L, WACenter);
	WMSetLabelFont(panel->dith2L, font);
	WMSetLabelText(panel->dith2L, _("More colors for\nWindow Maker"));

	WMMapSubwidgets(panel->dithF);

	WMRealizeWidget(panel->box);
	WMMapSubwidgets(panel->box);

	if (xis)
		RReleaseImage(xis);
	WMReleaseFont(font);
	showData(panel);
}

static void storeData(_Panel *panel)
{
	int i;

	for (i = 0; i < 5; i++) {
		if (WMGetButtonSelected(panel->icoB[i]))
			break;
	}
	SetSpeedForKey(i, "IconSlideSpeed");

	for (i = 0; i < 5; i++) {
		if (WMGetButtonSelected(panel->shaB[i]))
			break;
	}
	SetSpeedForKey(i, "ShadeSpeed");

	if (WMGetButtonSelected(panel->newsB)) {
		SetStringForKey("new", "NewStyle");
	} else if (WMGetButtonSelected(panel->oldsB)) {
		SetStringForKey("old", "NewStyle");
	} else {
		SetStringForKey("next", "NewStyle");
	}
	SetBoolForKey(!WMGetButtonSelected(panel->animB), "DisableAnimations");
	SetBoolForKey(WMGetButtonSelected(panel->supB), "Superfluous");
	SetBoolForKey(WMGetButtonSelected(panel->smoB), "SmoothWorkspaceBack");
	SetBoolForKey(WMGetButtonSelected(panel->dithB), "DisableDithering");
	SetIntegerForKey(WMGetSliderValue(panel->dithS), "ColormapSize");
}

Panel *InitConfigurations(WMScreen *scr, WMWidget *parent)
{
	_Panel *panel;

	panel = wmalloc(sizeof(_Panel));
	memset(panel, 0, sizeof(_Panel));

	panel->sectionName = _("Other Configurations");
	panel->description = _("Animation speeds, titlebar styles, various option\n"
			       "toggling and number of colors to reserve for\n" "Window Maker on 8bit displays.");

	panel->parent = parent;
	panel->callbacks.createWidgets = createPanel;
	panel->callbacks.updateDomain = storeData;

	AddSection(panel, ICON_FILE);

	return panel;
}
