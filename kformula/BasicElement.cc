/*
  BasicElement.cc
  Project KOffice/KFormula

  Author: Andrea Rizzi <rizzi@kde.org>
  License:GPL
*/

//#define RECT

#include <qrect.h>
#include "BasicElement.h"
#include "formuladef.h"
#include "kformula_doc.h"

BasicElement::BasicElement(KFormulaDoc *Formula,
			   BasicElement *Prev,
			   int Relation,
			   BasicElement *Next,
			   QString Content)
{
    formula=Formula;
    prev=Prev;
    relation=Relation;
    next=Next;
    content=Content;
    if(prev!=NULL) {
	numericFont=prev->getNumericFont();
	//warning("Font OK");
    } else
	numericFont=24;
    position=0;
    childrenNumber=0;
    minChildren=0;
    index[0]=0L;
    index[1]=0L;
    index[2]=0L;
    index[3]=0L;
    beActive=FALSE;
}

BasicElement::~BasicElement()
{
    /*ToDo:
     * Link prev with next & remove itself
     */
}

BasicElement *BasicElement::isInside(QPoint point)
{
    int i;
    if(myArea.contains(point)) {

	BasicElement *aValue=0L;
	if(next!=0)
	    if((aValue=next->isInside(point))!=0L)
		return aValue;
	
	for(i=0;i<4;i++)
	    if(index[i]!=0)
		if((aValue=index[i]->isInside(point))!=0L)
		    return aValue;
	
	for(i=0;i<childrenNumber;i++)
	    {
		if(child[i]!=0)
		    {
			warning("Child %i",i);
			if((aValue=child[i]->isInside(point))!=0L)
			    return aValue;
		    }
	    }
	return this;

    } else
	return 0L;
}

void BasicElement::draw(QPoint drawPoint,int resolution)
{
    QPainter *pen = formula->painter();
    //  QRect globalArea;
    int x = drawPoint.x();
    int y = drawPoint.y();
    if( beActive )
	pen->setPen(Qt::red);
    pen->setBrush(Qt::NoBrush);
    pen->drawRect(x+familySize.x(),y-5,10,10);

    myArea=globalSize;
    myArea.moveBy(x,y);
#ifdef RECT
    pen->drawRect(myArea);
#endif
    if(beActive)
	pen->setPen(Qt::blue);
    drawIndexes(pen,resolution);
    if(beActive)
	pen->setPen(Qt::black);
    if(next!=0L) next->draw(drawPoint+QPoint(localSize.width(),0),resolution);

 if(beActive){    
    if(position==0)
    formula->setCursor(QRect(x+familySize.x()-3,y-7,5,14));
    else
    formula->setCursor(QRect(x+familySize.x()+2,y-7,5,14));	
      }
}

void BasicElement::drawIndexes(QPainter *,int resolution)
{
    //draw point
    QPoint dp = myArea.topLeft()-globalSize.topLeft();

    if(index[0]!=0L)
	index[0]->draw(dp + familySize.topLeft() -
		       index[0]->getSize().bottomRight(),
		       resolution);
    if(index[1]!=0L)
	index[1]->draw(dp + familySize.bottomLeft() -
		       index[1]->getSize().topRight(),
		       resolution);
    if(index[2]!=0L)
	index[2]->draw(dp + familySize.topRight() -
		       index[2]->getSize().bottomLeft(),
		       resolution);
    if(index[3]!=0L)
	index[3]->draw(dp + familySize.bottomRight() -
		       index[3]->getSize().topLeft(),
		       resolution);
}

void BasicElement::checkSize()
{
    //warning("%p",this);
    QRect nextDimension;

    if (next!=0L)
	{
	    next->checkSize();
	    nextDimension=next->getSize();
	}
    localSize=QRect(0,-5,10,10);
    familySize=localSize;
    checkIndexesSize();  //This will change localSize adding Indexes Size
    familySize.moveBy(-localSize.left(),0);
    localSize.moveBy(-localSize.left(),0);
    globalSize=localSize;
    nextDimension.moveBy(localSize.width(),0);
    globalSize=globalSize.unite(nextDimension);

}

void BasicElement::check()
{
  int i=0; 
  for(i=0;i<childrenNumber;i++)
      if (child[i]==0L)
         if (i<minChildren)
	     child[i]=new BasicElement(formula,this,i+4);

}
void BasicElement::checkIndexesSize()
{
    QRect indexDimension;
    QPoint vectorT;
    int i;
    for(i=0;i<4;i++)
	if (index[i]!=0L) index[i]->checkSize();
    if(index[0]!=0L)
	{
	    indexDimension=index[0]->getSize();
	    vectorT=familySize.topLeft()-indexDimension.bottomRight();
	    indexDimension.moveBy(vectorT.x(),vectorT.y());
	    localSize=localSize.unite(indexDimension);
	}
    if(index[1]!=0L)
	{
	    indexDimension=index[1]->getSize();
	    vectorT=familySize.bottomLeft()-indexDimension.topRight();
	    indexDimension.moveBy(vectorT.x(),vectorT.y());
	    localSize=localSize.unite(indexDimension);
	}

    if(index[2]!=0L)
	{
	    indexDimension=index[2]->getSize();
	    vectorT=familySize.topRight()-indexDimension.bottomLeft();
	    indexDimension.moveBy(vectorT.x(),vectorT.y());
	    localSize=localSize.unite(indexDimension);
	}
    if(index[3]!=0L)
	{
	    indexDimension=index[3]->getSize();
	    vectorT=familySize.bottomRight()-indexDimension.topLeft();
	    indexDimension.moveBy(vectorT.x(),vectorT.y());
	    localSize=localSize.unite(indexDimension);
	}
}

void BasicElement::scaleNumericFont(int level)
{
    if((level & FN_ELEMENT)>0) {
	if ((level & FN_REDUCE)>0)
	    {
		if((level & FN_BYNUM)>0)
		    setNumericFont(numericFont-(level & 255));
		else  {
		    int den=(level & 15);
		    int num=((level>>4) & 15);
		    setNumericFont((numericFont*num)/den);
		}
	    } else {
		if((level & FN_BYNUM)>0)
		    setNumericFont(numericFont+(level & 255));
		else
		    {
			int num=(level & 15);
			int den=((level>>4) & 15);
			setNumericFont((numericFont*num)/den);
		    }
	    }
    }
    int ps;
    if(level & FN_INDEXES)
	for(ps=0;ps<4;ps++)
	    if(index[ps]!=0L) index[ps]->scaleNumericFont(level|FN_ALL);

    if(level & FN_NEXT)
	if(next!=0) next->scaleNumericFont(level|FN_ALL);

    if(level & FN_CHILDREN)
	{
	    for(ps=0;ps<childrenNumber;ps++)
		if(child[ps]!=0L)child[ps]->scaleNumericFont(level|FN_ALL);
	}
    if(numericFont<FN_MIN) numericFont=FN_MIN;
    if(numericFont>FN_MAX) numericFont=FN_MAX;
}


void  BasicElement::setNext(BasicElement *newNext)
{
    next=newNext;
}

void  BasicElement::setPrev(BasicElement *newPrev)
{
    prev=newPrev;
}

void BasicElement::setNumericFont(int value)
{
    numericFont=value;
}

int BasicElement::takeActionFromKeyb(int action)
{
if(action==Qt::Key_Delete)
 {
    warning("Key Delete");
    return  FCOM_DELETEME;
 }
if(action==Qt::Key_BackSpace)
  if(prev!=0)
    {
     formula->setActiveElement(prev);
     return  FCOM_DELETEME;    
    }
if(action==Qt::Key_Left)
    {
	int p=position-1;
	while((!isValidPosition(p))&&(p>0))
	            p--;
	      
       	setPosition(p);
    }
    
if(action==Qt::Key_Right)
    {
	int p=position+1;
	while((!isValidPosition(p)))
		   p++;
       	setPosition(p);
    }



 return 0;
}

void  BasicElement::substituteElement(BasicElement *clone)
{
    int i;
    clone->setContent(content);
    clone->setNext(next);
    clone->setPrev(prev);
    for(i=0;i<4;i++) {
	clone->setIndex(index[i],i);
	if(index[i]!=0L) index[i]->setPrev(clone);
    }
    /*  clone->setIndex(index[1],1);
	clone->setIndex(index[2],2);
	clone->setIndex(index[3],3);
    */
    clone->setNumericFont(numericFont);
    clone->setColor(defaultColor);
    clone->setRelation(relation);
    if(prev!=0L)
	{
	    if(relation<4)
		{
		    if(relation>=0)
			prev->setIndex(clone,relation);
		    else
			prev->setNext(clone);
		}
	    else
		prev->setChild(clone,relation-4);
	}
    else //I'm the first element!!
	formula->setFirstElement(clone);
    warning("Substituted %p with %p,  waiting to be deleted",this,clone);
}

int BasicElement::takeAsciiFromKeyb(char ch)
{
  return FCOM_ADDTEXT;    
}


void  BasicElement::insertElement(BasicElement *element)
{
    element->setPrev(prev);
    if(prev!=0L)
	{
	    if(relation<4)
		{
		    if(relation>=0)
			prev->setIndex(element,relation);
		    else
			prev->setNext(element);
		}
	    else
		prev->setChild(element,relation-4);
	}
    else //I'm the first element!!
	formula->setFirstElement(element);


    prev=element;
    element->setRelation(relation);
    element->setNext(this);
    relation=-1;
}

void  BasicElement::deleteElement(bool deleteme=true)
{
    warning("deleteElement of -> %p   prev %p    next %p",this,prev,next);
//    bool deleteme=true;
    if(next!=0L)
     {
     next->setPrev(prev);
     next->setRelation(relation);
     }
    if(prev!=0L)
     	{
	    if(relation<4)
		{
		    if(relation>=0)
			prev->setIndex(next,relation);
		    else
			prev->setNext(next);
		}
	    else
		prev->setChild(next,relation-4);
	}
    
    else //I'm the first element.
     if(next!=0L)
      formula->setFirstElement(next);    
     else
      formula->setFirstElement(new BasicElement(formula));    
//      deleteme=false;
      
   int nc=0;     
   while (nc<childrenNumber)
      { 
       if (child[nc]!=0L)
       {
       	warning("I'm %p, I delete my child[%d]=%p i.e. %d of %d",this,nc,child[nc],nc+1,childrenNumber);    
        child[nc]->deleteElement();
       }
        else 
	 nc++;
      }
   while (nc<4)
      { 
       if (index[nc]!=0L)
         index[nc]->deleteElement();
        else 
	 nc++;
      }
//if(deleteme)
 delete this;  // It is a good call ?
//else

}


void  BasicElement::save(ostream& out)
{
out << "TYPE=" << -1 << " "
    << "CONTENT=" << (const char *) content << " "
    << "NUMERICFONT=" << numericFont << " "
    << " >" << endl;

for(int i=0;i<4;i++)
 if(index[i]!=0L)
    {
     out << " <ELEM INDEX=" << i << " ";
     index[i]->save(out);     
    } 
    
for(int i=0;i<childrenNumber;i++)
 if(child[i]!=0L)
    {
     out << " <ELEM CHILD=" << i << " ";
     child[i]->save(out);     
    } 
if(next!=0)
    {
     out << " <ELEM NEXT ";
     next->save(out);     
    }         
out << "</ELEM>" << endl;
}

void  BasicElement::load(int)
{
}

bool BasicElement::isValidPosition(int pos) 
{
 
if((pos>0)&&(pos<=4))
 return (index[pos-1]!=0);

if((pos>4)&&(pos<=4+childrenNumber))
 return (child[pos-5]!=0);

if(pos<-3)
return false;

if(pos>6+childrenNumber)
return false;

return true;
}

void BasicElement::setPosition(int pos) 
{    
   position=pos; 
   warning("setPosition %d of element %p",pos,this);

   if(position==-2)
     position=childrenNumber+5;   //Go to end
   
   if(position==-1)		 //Go to prev
    {
     if(prev!=0)
        { 
         formula->setActiveElement(prev);  
         warning("Set active %p",prev);
	 if(relation==-1)
   	  prev->setPosition(-2);
	 if(relation>=0)
	 {
	 int p=relation;	 
	 while((!prev->isValidPosition(p))&&(p>0))
	       p--;
   	  prev->setPosition(p);
         }
	}
	else
	position=0;
     }  

  if((position>0) && (position<=4))
    {
      formula->setActiveElement(index[position-1]);  
              warning("Set active %p",index[position-1]);
      index[position-1]->setPosition(0);
     }
    
   if((position>4) && (position<=4+childrenNumber))
    {
      formula->setActiveElement(child[position-5]);  
        warning("Set active %p",child[position-5]);
      child[position-5]->setPosition(0);
    }
  
   if(position>childrenNumber+5)   //if childrenNumber+5 do nothing.
      {
       if(next!=0)
        { 
         formula->setActiveElement(next);  
          warning("Set active %p",next);
	 next->setPosition(0);
        }
  	 else
	{
	   warning("no next");
	   if(prev!=0)
    	    position=-3;
	    
	  else
	    position=childrenNumber+5; 
        }
       }
    // }

     if(position==-3)
        {
	  if(prev!=0)
	  {
           formula->setActiveElement(prev);  
  	 warning("Set active %p",prev);
	    if(relation==-1)
	      prev->setPosition(-3);
	     else
	      {
	         int p=relation+2;	 
	         while((!prev->isValidPosition(p)))
	           p++;
	      
       	     prev->setPosition(p);
	    }
	   }
	    else
     	     position=0;
	 }
   warning("END OF setPosition %d of Element %p",position,this);
}