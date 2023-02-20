import time

global size
size=5

class pos:
    def __init__(self,x,y) -> None:
        self.x = x
        self.y = y

    def display(self):
        print(self.x,self.y)

class cell:
    def __init__(self, r, c, color_):
        self.color = color_ # 0 indicates (traversing is possible) and -1 indicates (obstruction)
        self.is_visited=False
        self.row = r
        self.col = c #assume the row and column starts from 0 to n-1
        self.directions =[] #stores in lrbt
        self.costs=None
        if self.color == -1 :
            self.dijkstra_num=-1
        else:
            self.dijkstra_num=0
        

class grid:
    def __init__(self):
        self.track_stack=[]
        self.mat=[
            [ cell(0,0, 0)  , cell(0,0,-1)  , cell(0,0, 0) ,  cell(0,0, 0)  , cell(0,0,0) ],
            [ cell(0,0, 0)  , cell(0,0,-1)  , cell(0,0, 0)  , cell(0,0, 0)  , cell(0,0,0) ],
            [ cell(0,0, 0)  , cell(0,0,-1)  , cell(0,0, 0)  , cell(0,0,-1)  , cell(0,0,0) ],
            [ cell(0,0, 0)  , cell(0,0, 0)  , cell(0,0, 0)  , cell(0,0, 0)  , cell(0,0,0) ],
            [ cell(0,0, 0)  , cell(0,0, 0)  , cell(0,0, 0)  , cell(0,0, 0)  , cell(0,0,0) ],
            ]

        for i in range(size):
            for j in range(size):
                self.mat[i][j].directions=self.update_directions(i,j)
    
    def check_cell_exists(self,r,c):
        return ( r >= 0 and c >= 0 and r<size and c<size)
    
    def available(self,r,c):
        return ( self.check_cell_exists(r,c) and (self.mat[r][c].color !=-1) )

    def update_directions(self,r,c):
        dir=[]
        #left
        if (self.check_cell_exists(r,c-1) and self.mat[r][c-1].color!=-1) :
            dir.append(True)
        else:
            dir.append(False)
        #right
        if (self.check_cell_exists(r,c+1) and self.mat[r][c+1].color!=-1) :
            dir.append(True)
        else:
            dir.append(False)
        #bottom
        if (self.check_cell_exists(r+1,c) and self.mat[r+1][c].color!=-1) :
            dir.append(True)
        else:
            dir.append(False)
        #top
        if (self.check_cell_exists(r-1,c) and self.mat[r-1][c].color!=-1) :
            dir.append(True)
        else:
            dir.append(False)
        return dir

    def display_dijkstra(self):
        print("The matrix is after me: ")
        for i in range(size):
            for j in range(size):
                d=self.mat[i][j].dijkstra_num
                if ( d ==-1) :
                    print("X",end=" ")
                else:
                    print(d,end=" ")
            print("\n")
    
    def calc_frontier(self, r, c):
        frontier=[0,0,0,0]
        curr =self.mat[r][c]
        if self.check_cell_exists(r,c-1):
            next =self.mat[r][c-1]
            if next.is_visited:
                if ((curr.dijkstra_num - next.dijkstra_num) > 1) or ((curr.dijkstra_num - next.dijkstra_num) < -1):
                    frontier[0]= curr.dijkstra_num-next.dijkstra_num

        if self.check_cell_exists(r,c+1):
            next =self.mat[r][c+1]
            if next.is_visited:
                if ((curr.dijkstra_num - next.dijkstra_num) > 1) or ((curr.dijkstra_num - next.dijkstra_num) < -1):
                    frontier[1]= curr.dijkstra_num-next.dijkstra_num

        if self.check_cell_exists(r+1,c):
            next =self.mat[r+1][c]
            if next.is_visited:
                if ((curr.dijkstra_num - next.dijkstra_num) > 1) or ((curr.dijkstra_num - next.dijkstra_num) < -1):
                    frontier[2]= curr.dijkstra_num-next.dijkstra_num

        if self.check_cell_exists(r-1,c):
            next =self.mat[r-1][c]
            if next.is_visited:
                if ((curr.dijkstra_num - next.dijkstra_num) > 1) or ((curr.dijkstra_num - next.dijkstra_num) < -1):
                    frontier[3]= curr.dijkstra_num-next.dijkstra_num
        return frontier

    def fill_dijkstra(self,pos_beg,num):
        time.sleep(1)
        x = pos_beg.x
        y = pos_beg.y
        self.display_dijkstra()
        if (x>=0 and x<size and y<size and y>=0):
            self.mat[x][y].dijkstra_num=num
            self.mat[x][y].is_visited=True

            surr=[]
            if self.available(x,y-1):
                next=self.mat[x][y-1]
                if not next.is_visited :
                    self.mat[x][y-1].dijkstra_num =num+1
                    self.mat[x][y-1].is_visited=True
                    surr.append(pos(x,y-1))

            if self.available(x,y+1):
                next=self.mat[x][y+1]
                if not next.is_visited :
                    self.mat[x][y+1].dijkstra_num =num+1
                    self.mat[x][y+1].is_visited=True
                    surr.append(pos(x,y+1))
                    
            if self.available(x+1,y):
                next=self.mat[x+1][y]
                if not next.is_visited :
                    self.mat[x+1][y].dijkstra_num =num+1
                    self.mat[x+1][y].is_visited=True
                    surr.append(pos(x+1,y))

            if self.available(x-1,y):
                next=self.mat[x-1][y]
                if not next.is_visited :
                    self.mat[x-1][y].dijkstra_num =num+1
                    self.mat[x-1][y].is_visited=True
                    surr.append(pos(x-1,y))

            frontier =self.calc_frontier(x,y)

            #left
            if (frontier[0]<-1):
                self.fill_dijkstra(pos(x,y-1),self.mat[x][y].dijkstra_num+1)
            elif (frontier[0]>1):
                self.fill_dijkstra(pos(x,y),self.mat[x][y-1].dijkstra_num+1)
            else:
                pass
            frontier =self.calc_frontier(x,y)
            #right
            if (frontier[1]<-1):
                self.fill_dijkstra(pos(x,y+1),self.mat[x][y].dijkstra_num+1)
            elif (frontier[1]>1):
                self.fill_dijkstra(pos(x,y),self.mat[x][y+1].dijkstra_num+1)
            else:
                pass
            frontier =self.calc_frontier(x,y)
            #bottom
            if (frontier[2]<-1):
                self.fill_dijkstra(pos(x+1,y),self.mat[x][y].dijkstra_num+1)
            elif (frontier[2]>1):
                self.fill_dijkstra(pos(x,y),self.mat[x+1][y].dijkstra_num+1)
            else:
                pass
            frontier =self.calc_frontier(x,y)
            #top
            if (frontier[3]<-1):
                self.fill_dijkstra(pos(x-1,y),self.mat[x][y].dijkstra_num+1)
            elif (frontier[3]>1):
                self.fill_dijkstra(pos(x,y),self.mat[x-1][y].dijkstra_num+1)
            else:
                pass
            self.display_dijkstra()

            if surr ==[] :
                return
            else:
                for i in range(len(surr)):
                    x_= surr[i].x
                    y_= surr[i].y
                    cell_info =self.mat[x_][y_]
                    self.fill_dijkstra(pos(x_,y_),num+1)
        else:
            return    
    
    def track(self,pos_start,pos_dest):
        x = pos_dest.x
        y = pos_dest.y
        x_ = pos_start.x
        y_ = pos_start.y
        final_dijkstra = self.mat[x_][y_].dijkstra_num
        curr_dijkstra=self.mat[x][y].dijkstra_num
        self.track_stack.append(pos(x,y))
        print("Position: ",x,y," Dijkstra: ",curr_dijkstra)
        while (curr_dijkstra!=final_dijkstra) :
            time.sleep(1)
            if self.available(x,y-1):
                next = self.mat[x][y-1]
                if next.dijkstra_num == curr_dijkstra-1:
                    y -=1
                    x =x
                    
            if self.available(x,y+1):
                next=self.mat[x][y+1]
                if next.dijkstra_num == curr_dijkstra-1:
                    y +=1
                    x=x

            if self.available(x+1,y):
                next=self.mat[x+1][y]
                if next.dijkstra_num == curr_dijkstra-1:
                    x+=1
                    y=y

            if self.available(x-1,y):
                next=self.mat[x-1][y]
                if next.dijkstra_num == curr_dijkstra-1:
                    x-=1
                    y=y
            self.track_stack.append(pos(x,y))
            curr_dijkstra = self.mat[x][y].dijkstra_num
            print("Position: ",x,y," Dijkstra: ",curr_dijkstra)
        print("In order")
        while(self.track_stack!=[]):
            self.track_stack.pop().display()




if __name__ =="__main__":
    maze= grid()
    maze.fill_dijkstra(pos(2,2),0)
    print("Complete")
    maze.track(pos(2,2),pos(0,0))