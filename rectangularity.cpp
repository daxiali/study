#ifndef __BEAN_RECTANGULARITY_HPP__
#define __BEAN_RECTANGULARITY_HPP__

#include <iostream>
#include <limits>
#include <array>
#include <vector>
#include <set>
#include <map>

struct Point
{
    int x, y;

    Point() {}
    Point(int x, int y)
    {
        this->x = x;
        this->y = y;
    }
};

struct Edge
{
    Point head, tail;

    Edge() {}
    Edge(Point const &head, Point const &tail)
    {
        this->head = head;
        this->tail = tail;
    }

    inline int getMinX() const { return head.x < tail.x ? head.x : tail.x; }
    inline int getMinY() const { return head.y < tail.y ? head.y : tail.y; }
    inline int getMaxX() const { return head.x > tail.x ? head.x : tail.x; }
    inline int getMaxY() const { return head.y > tail.y ? head.y : tail.y; }
    inline bool isHorizontal() const { return head.y == tail.y; }
    inline bool isVertical() const { return head.x == tail.x; }
};

void getScanLine(const std::vector<Point> &poly, std::set<int> &scan_lines)
{
    int max_y = std::numeric_limits<int>::min();
    for (size_t i = 0; i < poly.size(); i++) {
        Edge e(poly[i], poly[(i+1) % poly.size()]);
        if (e.isHorizontal())
            continue;
        scan_lines.insert(e.getMinY());

        int y = e.getMaxY();
        if (y > max_y)
            max_y = y;
    }

    scan_lines.insert(max_y);
}

void addEdgePoints(const std::vector<Point> &poly, std::set<int> &scan_lines, std::map<int, std::set<int>> &fracture_points)
{
    for (size_t i = 0; i < poly.size(); i++) {
        Edge e(poly[i], poly[(i+1) % poly.size()]);
        if (e.isHorizontal())
            continue;
        
        int y1 = e.getMinY();
        int y2 = e.getMaxY();

        auto it = scan_lines.find(y1);
        while (*it < y2) {
            fracture_points[*it].insert(poly[i].x);
            ++it;
        }
    }
}

void createRectangles(std::map<int, std::set<int>> &fracture_points, std::set<int> &scan_lines, std::vector<std::array<Point, 4>> &rects)
{
    for (auto it = fracture_points.begin(); it != fracture_points.end(); ++it) {
        int y1 = it->first;
        auto &xs = it->second;

        auto sit = scan_lines.find(y1);
        int y2 = *(++sit);
        
        for (auto pit = xs.begin(); pit != xs.end(); ++pit) {
            int x1 = *pit;
            int x2 = *(++pit);

            std::array<Point, 4> r;
            r[0] = Point(x1, y1);
            r[1] = Point(x2, y1);
            r[2] = Point(x2, y2);
            r[3] = Point(x1, y2);

            rects.emplace_back(std::move(r));
        }
    }
}

void doRectangularityPolygon(const std::vector<Point> &poly, std::vector<std::array<Point, 4>> &rects)
{
    std::set<int> scan_lines;
    getScanLine(poly, scan_lines);

    std::map<int, std::set<int>> fracture_points;
    addEdgePoints(poly, scan_lines, fracture_points);

    createRectangles(fracture_points, scan_lines, rects);
}

void printRectangles(std::vector<std::array<Point, 4>> &rects)
{
    for (size_t i = 0; i < rects.size(); i++) {
        std::cout << "rectangle " << i << ": ";
        auto &r = rects[i];
        for (size_t j = 0; j < 4; j++)
            std::cout << "(" << r[j].x << ", " << r[j].y << ") ";

        std::cout << std::endl;
    }
}

int main()
{
    std::vector<Point> poly = {
        {5, 5}, {10, 5}, {10, 8}, {14, 8}, {14, 14}, {17, 14}, {17, 7}, {20, 7},
        {20, 21}, {18, 21}, {18, 19}, {15, 19}, {15, 20}, {10, 20}, {10, 18}, {7, 18}, {7, 16}, {6, 16}, {6, 19}, {5, 19}
    };

    std::vector<std::array<Point, 4>> rects;
    doRectangularityPolygon(poly, rects);

    printRectangles(rects);

    return 0;
}

#endif // __BEAN_RECTANGULARITY_HPP