// 
//---------------------------------------------------------------------------
//
// Copyright(C) 2002-2016 Christoph Oelckers
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
//--------------------------------------------------------------------------
//
/*
** gl_drawinfo.cpp
** Implements the draw info structure which contains most of the
** data in a scene and the draw lists - including a very thorough BSP 
** style sorting algorithm for translucent objects.
**
*/

#include "gl_load/gl_system.h"
#include "r_sky.h"
#include "r_utility.h"
#include "doomstat.h"
#include "g_levellocals.h"
#include "tarray.h"
#include "hwrenderer/scene/hw_drawstructs.h"
#include "hwrenderer/data/flatvertices.h"
#include "hwrenderer/utility/hw_clock.h"

#include "gl/scene/gl_drawinfo.h"
#include "hwrenderer/scene/hw_clipper.h"
#include "gl/renderer/gl_renderstate.h"
#include "gl/renderer/gl_renderer.h"
#include "hwrenderer/data/hw_viewpointbuffer.h"
#include "hwrenderer/dynlights/hw_lightbuffer.h"
#include "hwrenderer/models/hw_models.h"

bool FDrawInfo::SetDepthClamp(bool on)
{
	return gl_RenderState.SetDepthClamp(on);
}

//==========================================================================
//
//
//
//==========================================================================

static int dt2gl[] = { GL_POINTS, GL_LINES, GL_TRIANGLES, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP };

void FDrawInfo::Draw(EDrawType dt, FRenderState &state, int index, int count, bool apply)
{
	assert(&state == &gl_RenderState);
	if (apply)
	{
		gl_RenderState.Apply();
	}
	drawcalls.Clock();
	glDrawArrays(dt2gl[dt], index, count);
	drawcalls.Unclock();
}

void FDrawInfo::DrawIndexed(EDrawType dt, FRenderState &state, int index, int count, bool apply)
{
	assert(&state == &gl_RenderState);
	if (apply)
	{
		gl_RenderState.Apply();
	}
	drawcalls.Clock();
	glDrawElements(dt2gl[dt], count, GL_UNSIGNED_INT, (void*)(intptr_t)(index * sizeof(uint32_t)));
	drawcalls.Unclock();
}

void FDrawInfo::RenderPortal(HWPortal *p, bool usestencil)
{
	auto gp = static_cast<HWPortal *>(p);
	gp->SetupStencil(this, gl_RenderState, usestencil);
	auto new_di = StartDrawInfo(Viewpoint, &VPUniforms);
	new_di->mCurrentPortal = gp;
	gl_RenderState.SetLightIndex(-1);
	gp->DrawContents(new_di, gl_RenderState);
	new_di->EndDrawInfo();
	screen->mVertexData->Bind(gl_RenderState);
	screen->mViewpoints->Bind(this, vpIndex);
	gp->RemoveStencil(this, gl_RenderState, usestencil);

}

void FDrawInfo::SetDepthMask(bool on)
{
	glDepthMask(on);
}

void FDrawInfo::SetDepthFunc(int func)
{
	static int df2gl[] = { GL_LESS, GL_LEQUAL, GL_ALWAYS };
	glDepthFunc(df2gl[func]);
}

void FDrawInfo::SetDepthRange(float min, float max)
{
	glDepthRange(min, max);
}

void FDrawInfo::EnableDrawBufferAttachments(bool on)
{
	gl_RenderState.EnableDrawBuffers(on? gl_RenderState.GetPassDrawBufferCount() : 1);
}

void FDrawInfo::SetStencil(int offs, int op, int flags)
{
	static int op2gl[] = { GL_KEEP, GL_INCR, GL_DECR };

	glStencilFunc(GL_EQUAL, screen->stencilValue + offs, ~0);		// draw sky into stencil
	glStencilOp(GL_KEEP, GL_KEEP, op2gl[op]);		// this stage doesn't modify the stencil

	bool cmon = !(flags & SF_ColorMaskOff);
	glColorMask(cmon, cmon, cmon, cmon);						// don't write to the graphics buffer
	glDepthMask(!(flags & SF_DepthMaskOff));
	if (flags & SF_DepthTestOff)
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);
	if (flags & SF_DepthClear)
		glClear(GL_DEPTH_BUFFER_BIT);
}

void FDrawInfo::SetCulling(int mode)
{
	if (mode != Cull_None)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(mode == Cull_CCW ? GL_CCW : GL_CW);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}

void FDrawInfo::EnableClipDistance(int num, bool state)
{
	// Update the viewpoint-related clip plane setting.
	if (!(gl.flags & RFL_NO_CLIP_PLANES))
	{
		if (state)
		{
			glEnable(GL_CLIP_DISTANCE0+num);
		}
		else
		{
			glDisable(GL_CLIP_DISTANCE0+num);
		}
	}
}

//==========================================================================
//
//
//
//==========================================================================
void FDrawInfo::ClearScreen()
{
	bool multi = !!glIsEnabled(GL_MULTISAMPLE);

	screen->mViewpoints->Set2D(this, SCREENWIDTH, SCREENHEIGHT);
	gl_RenderState.SetColor(0, 0, 0);
	gl_RenderState.Apply();

	glDisable(GL_MULTISAMPLE);
	glDisable(GL_DEPTH_TEST);

	glDrawArrays(GL_TRIANGLE_STRIP, FFlatVertexBuffer::FULLSCREEN_INDEX, 4);

	glEnable(GL_DEPTH_TEST);
	if (multi) glEnable(GL_MULTISAMPLE);
}


