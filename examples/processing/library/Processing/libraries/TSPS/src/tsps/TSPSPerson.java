package tsps;

import processing.core.PVector;
import java.util.ArrayList;

public class TSPSPerson
{
	
	public int id; 
	public int age; 
	public PVector centroid;  
	public PVector velocity; 
	public PVector opticalFlow; 
	public Rectangle boundingRect;
	public boolean dead;
	
	public final ArrayList<PVector> contours;
	
	public TSPSPerson(){
		boundingRect = new Rectangle();
		centroid = new PVector();
		velocity = new PVector();
		opticalFlow = new PVector();
		dead = false;
		contours = new ArrayList<PVector>();
	}
	
	public void update ( TSPSPerson p )
	{
		age = p.age;
		boundingRect = p.boundingRect;
		centroid = p.centroid;
		velocity = p.velocity;
		opticalFlow = p.opticalFlow;

		synchronized (contours)
		{
			contours.clear();
			for (PVector point : contours)
			{
				contours.add(point);
			}
		}
	}
}