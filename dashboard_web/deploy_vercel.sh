#!/bin/bash
# Enterprise Deploy Script for Vercel
# Execute this inside the `/dashboard_web` directory.

echo "Building Flutter Web Dashboard for Production..."
flutter build web --release

echo "Linking Vercel Project..."
# (Skip this if you have already run `vercel link`)
vercel link --yes

echo "Deploying to Production..."
vercel --prod

echo "Assigning Custom Domain..."
# Replace 'dashboard.myenterprise.com' with your actual domain
vercel alias set dashboard.myenterprise.com
