//
//  NSBezierPath+CGPath.hpp
//  Dock Tile
//
//  Created by Eric Marchand on 24/07/2021.
//

#ifndef DOCK_PATH_H
#define DOCK_PATH_H

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@interface NSBezierPath (CGPath)

@property (readonly, getter=quartzPath) CGPathRef CGPath ;

-(CGPathRef)quartzPath;

@end

@interface ProgressCircleShapeLayer : CAShapeLayer

- (instancetype)initWithRadius:(CGFloat)radius center:(CGPoint)center;
- (void)setProgress:(CGFloat)progress;

@end

@interface ProgressSquircleShapeLayer : CAShapeLayer

- (instancetype)initWithRect:(CGRect)rect;
- (void)setProgress:(CGFloat)progress;

@end


#endif /* DOCK_PATH_H */
