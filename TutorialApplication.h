#ifndef __TutorialApplication_h_
#define __TutorialApplication_h_

#include "BaseApplication.h"

class BasicTutorial2 : public BaseApplication
{
public:
	BasicTutorial2(void);
	virtual ~BasicTutorial2(void);
 
protected:
	virtual void createScene(void);
	virtual void createCamera(void);
	virtual void createViewports(void);
};

#endif // #ifndef __TutorialApplication_h_
