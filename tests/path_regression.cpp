// Header self-containment is intentional: it must be the first include.
#include "ViennaGameAILibrary.hpp"
#include <stdexcept>

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
double length(const std::vector<VGAIL::Vec2ui>& p) {
    double sum=0;
    for (std::size_t i=1;i<p.size();++i) {
        require(!(p[i]==p[i-1]), "duplicate adjacent waypoint");
        const double dx=double(p[i].x)-p[i-1].x,dy=double(p[i].y)-p[i-1].y;
        require(std::abs(dx)<=1 && std::abs(dy)<=1,"nonadjacent waypoint");
        sum+=std::hypot(dx,dy);
    }
    return sum;
}
// Independent Dijkstra on the same corner-cutting 8-neighbor graph.
double oracle(VGAIL::NavMesh& mesh,unsigned n,VGAIL::Vec2ui s,VGAIL::Vec2ui t) {
    const double inf=std::numeric_limits<double>::infinity();
    if(mesh.getNode(s).state==VGAIL::NodeState::OBSTRUCTABLE ||
       mesh.getNode(t).state==VGAIL::NodeState::OBSTRUCTABLE)return inf;
    std::vector<double>d(n*n,inf);d[s.y*n+s.x]=0;
    using E=std::pair<double,unsigned>;
    std::priority_queue<E,std::vector<E>,std::greater<E>>q;q.push({0,s.y*n+s.x});
    while(!q.empty()) {
        auto[c,i]=q.top();q.pop();if(c!=d[i])continue;if(i==t.y*n+t.x)return c;
        int x=i%n,y=i/n;
        for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){
            int u=x+dx,v=y+dy;if((!dx&&!dy)||u<0||v<0||u>=int(n)||v>=int(n))continue;
            if(mesh.getNode({unsigned(u),unsigned(v)}).state==VGAIL::NodeState::OBSTRUCTABLE)continue;
            unsigned j=v*n+u;double next=c+std::hypot(dx,dy);
            if(next<d[j]){d[j]=next;q.push({next,j});}
        }
    }return inf;
}
int main() {
    VGAIL::NavMesh empty(10,10,0,2,2);empty.preprocess();
    auto p=empty.findPreprocessedPath({0,0},{2,1});
    require(std::abs(length(p)-(1+std::sqrt(2.)))<1e-5,"original detour regression");
    require(!empty.findPreprocessedPath({0,0},{10,1}).size(),"invalid endpoint");
    std::mt19937 rng(20261029);unsigned queries=0;
    for(unsigned trial=0;trial<5;++trial) {
        constexpr unsigned n=6;VGAIL::NavMesh m(n,n,0,3,3);
        for(unsigned y=0;y<n;++y)for(unsigned x=0;x<n;++x)
            if(rng()%5==0)m.getNode({x,y}).state=VGAIL::NodeState::OBSTRUCTABLE;
        m.preprocess(trial%2==1,2); // exercise both preprocessing paths
        for(unsigned sy=0;sy<n;++sy)for(unsigned sx=0;sx<n;++sx)
         for(unsigned ty=0;ty<n;++ty)for(unsigned tx=0;tx<n;++tx){
            const VGAIL::Vec2ui s{sx,sy},t{tx,ty};double expected=oracle(m,n,s,t);
            for(auto route:{m.findPath(s,t),m.findPreprocessedPath(s,t)}){
                require(route.empty()==!std::isfinite(expected),"reachability mismatch");
                if(!route.empty()) {
                    require(route.front()==s && route.back()==t,"wrong endpoints");
                    for(auto point:route)require(m.getNode(point).state!=VGAIL::NodeState::OBSTRUCTABLE,"blocked waypoint");
                    require(std::abs(length(route)-expected)<1e-4,"nonoptimal path");
                }
            }++queries;
        }
        // A stale cached route must not pass through a newly blocked cell.
        m.getNode({3,3}).state=VGAIL::NodeState::OBSTRUCTABLE;
        auto route=m.findPreprocessedPath({0,0},{5,5});
        double expected=oracle(m,n,{0,0},{5,5});
        require(route.empty()==!std::isfinite(expected),"stale-cache reachability");
        if(!route.empty())require(std::abs(length(route)-expected)<1e-4,"stale cache");
        bool rejected=false;try{m.preprocess(true,0);}catch(const std::invalid_argument&){rejected=true;}
        require(rejected,"zero worker count");
    }
    std::cout<<"PASS: "<<queries<<" endpoint pairs, A* and cached queries vs Dijkstra; header, detour, join, stale cache\n";
}
