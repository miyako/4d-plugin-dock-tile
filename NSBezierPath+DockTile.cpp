//
//  NSBezierPath+DockTile.cpp
//  Dock Tile
//
//  Created by Eric Marchand on 24/07/2021.
//
#import "NSBezierPath+DockTile.h"

@implementation NSBezierPath (CGPath)

- (CGPathRef)quartzPath
{
    NSInteger i, numElements;
 
    // Need to begin a path here.
    CGPathRef           immutablePath = NULL;
 
    // Then draw the path elements.
    numElements = [self elementCount];
    if (numElements > 0)
    {
        CGMutablePathRef    path = CGPathCreateMutable();
        NSPoint             points[3];
        BOOL                didClosePath = YES;
        
        for (i = 0; i < numElements; i++)
        {
            switch ([self elementAtIndex:i associatedPoints:points])
            {
                case NSMoveToBezierPathElement:
                    CGPathMoveToPoint(path, NULL, points[0].x, points[0].y);
                    break;
                    
                case NSLineToBezierPathElement:
                    CGPathAddLineToPoint(path, NULL, points[0].x, points[0].y);
                    didClosePath = NO;
                    break;
                    
                case NSCurveToBezierPathElement:
                    CGPathAddCurveToPoint(path, NULL, points[0].x, points[0].y,
                                          points[1].x, points[1].y,
                                          points[2].x, points[2].y);
                    didClosePath = NO;
                    break;
                    
                case NSClosePathBezierPathElement:
                    CGPathCloseSubpath(path);
                    didClosePath = YES;
                    break;
            }
        }
        
        // Be sure the path is closed or Quartz may not do valid hit detection.
        if (!didClosePath)
            CGPathCloseSubpath(path);
        
        immutablePath = CGPathCreateCopy(path);
        CGPathRelease(path);
    }
 
    return immutablePath;
}

@end

@implementation ProgressCircleShapeLayer
- (void)setProgress:(CGFloat)progress {
    self.strokeEnd = CGFloat(progress * 1.02);
}
 - (instancetype)initWithRadius:(CGFloat)radius center:(CGPoint)center {
     if ( !(self = [super init]) )
     {
         return nil;
     }
     self.fillColor = nil;
     self.lineCap = @"kCALineCapRound";
     self.position = center;
     self.strokeEnd = 0;

     CGFloat startAngle = 90;
     NSBezierPath* path = [[NSBezierPath alloc] init];
     [path appendBezierPathWithArcWithCenter:center radius:radius startAngle:startAngle endAngle:startAngle - 360 clockwise:YES];
     [path closePath];

     CGPathRef pathRef = path.CGPath;
     self.path = pathRef;
     self.bounds = CGPathGetBoundingBox(pathRef);

     return self;
 }
@end

@implementation ProgressSquircleShapeLayer
- (void)setProgress:(CGFloat)progress {
    self.strokeEnd = CGFloat(progress * 1.02);
}
 - (instancetype)initWithRect:(CGRect)rect {
     if ( !(self = [super init]) )
     {
         return nil;
     }
     self.fillColor = nil;
     self.lineCap = @"kCALineCapRound";
     self.position = CGPointZero;
     self.strokeEnd = 0;

     NSBezierPath* path = [[NSBezierPath alloc] init];

     CGFloat cornerRadius = rect.size.width / 2;
     CGFloat minSide = fmin(rect.size.width, rect.size.height);
     CGFloat radius = fmin(CGFloat(cornerRadius), minSide / 2);

     CGPoint center = CGPointMake(CGRectGetMidX(rect), CGRectGetMidY(rect));
     CGPoint topLeft = CGPointMake(CGRectGetMinX(rect), CGRectGetMinY(rect));
     CGPoint topRight = CGPointMake(CGRectGetMaxX(rect) ,CGRectGetMinY(rect));
     CGPoint bottomLeft = CGPointMake(CGRectGetMinX(rect), CGRectGetMaxY(rect));
     CGPoint bottomRight = CGPointMake(CGRectGetMaxX(rect) ,CGRectGetMaxY(rect));
     CGPoint point1 = CGPointMake(CGRectGetMinX(rect) + radius, CGRectGetMinY(rect));
     CGPoint point2 = CGPointMake(CGRectGetMaxX(rect) - radius, CGRectGetMinY(rect));
     CGPoint point3 = CGPointMake(CGRectGetMaxX(rect), CGRectGetMinY(rect) + radius);
     CGPoint point4 = CGPointMake(CGRectGetMaxX(rect), CGRectGetMaxY(rect) - radius);
     CGPoint point5 = CGPointMake(CGRectGetMaxX(rect) - radius, CGRectGetMaxY(rect));
     CGPoint point6 = CGPointMake(CGRectGetMinX(rect) + radius, CGRectGetMaxY(rect));
     CGPoint point7 = CGPointMake(CGRectGetMinX(rect) , CGRectGetMaxY(rect) - radius);
     CGPoint point8 = CGPointMake(CGRectGetMinX(rect) , CGRectGetMinY(rect) + radius);

     [path moveToPoint: point1];
     [path lineToPoint: point2];
     [path curveToPoint:point3 controlPoint1:topRight controlPoint2:topRight];
     [path lineToPoint: point4];
     [path curveToPoint: point5 controlPoint1: bottomRight controlPoint2: bottomRight];
     [path lineToPoint: point6];
     [path curveToPoint: point7 controlPoint1: bottomLeft controlPoint2: bottomLeft];
     [path lineToPoint: point8];
     [path curveToPoint: point1 controlPoint1: topLeft controlPoint2: topLeft];

     NSAffineTransform* transform = [[NSAffineTransform alloc] init];

     [transform translateXBy: center.x yBy: center.y];
     [transform rotateByDegrees: 90];
     [transform translateXBy: -center.x yBy: -center.y];
     [path transformUsingAffineTransform: transform];
     path = [path bezierPathByReversingPath];

     CGPathRef pathRef = path.CGPath;
     self.path = pathRef;
     self.bounds = CGPathGetBoundingBox(pathRef);

     return self;
}
@end
