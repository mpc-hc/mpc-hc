/*
* (C) 2014 see Authors.txt
*
* This file is part of MPC-HC.
*
* MPC-HC is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
*
* MPC-HC is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#pragma once

#include <vector>
#include <memory>
#include <atlcoll.h>

class CEllipse
{
public:
    enum {
        NO_INTERSECT_OUTER = INT_MIN,
        NO_INTERSECT_INNER = INT_MAX,
    };

private:
    static int const NOT_CACHED = NO_INTERSECT_OUTER + 1;

    int m_rx = -1, m_ry = -1;
    int m_2rx, m_2ry;

    std::vector<int> m_arc;

    std::vector<int> m_intersectCache;
    size_t nIntersectCacheLineSize;

public:
    CEllipse(int rx, int ry);

    int GetXRadius() const {
        return m_rx;
    }

    int GetYRadius() const {
        return m_ry;
    }

    int GetArc(int dy) const {
        return m_arc[m_ry + dy];
    }

    int GetLeftIntersect(int dx, int dy);

    int GetRightIntersect(int dx, int dy) {
        return GetLeftIntersect(-dx, dy);
    }
};

typedef std::shared_ptr<CEllipse> CEllipseSharedPtr;

struct EllipseCenter {
    int x, y;
    int yStopDrawing;
};

struct SpanEndPoint {
    int x;
    bool bEnd;

    SpanEndPoint(int x, bool bEnd)
        : x(x)
        , bEnd(bEnd) {}

    bool operator<(const SpanEndPoint& right) const {
        if (x == right.x) {
            return (!bEnd && right.bEnd);
        } else {
            return x < right.x;
        }
    }
};

class CEllipseCenterGroup
{
    const CEllipseSharedPtr& m_pEllipse;
    CAtlList<EllipseCenter> m_leftCenters, m_rightCenters;

    template<typename IntersectFunction>
    void AddPoint(CAtlList<EllipseCenter>& centers, IntersectFunction intersect, int x, int y);

public:
    enum Position {
        BEFORE,
        INSIDE,
        AFTER
    };

    CEllipseCenterGroup(const CEllipseSharedPtr& pEllipse)
        : m_pEllipse(pEllipse) {}

    CEllipseCenterGroup(const CEllipseCenterGroup& ellipseGroup)
        : m_pEllipse(ellipseGroup.m_pEllipse) {
        m_leftCenters.AddHeadList(&ellipseGroup.m_leftCenters);
        m_rightCenters.AddHeadList(&ellipseGroup.m_rightCenters);
    }

    bool IsEmpty() const {
        ASSERT(m_leftCenters.IsEmpty() == m_rightCenters.IsEmpty());
        return m_leftCenters.IsEmpty();
    }

    Position GetRelativePosition(int xLeft, int y);

    void AddSpan(int y, int xLeft, int xRight);

    void FlushLine(int y, std::vector<SpanEndPoint>& wideSpanEndPoints);
};
