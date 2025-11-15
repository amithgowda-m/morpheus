#!/usr/bin/env python3
"""
Morpheus Benchmark Dashboard Server
Real-time WebSocket updates for live benchmark monitoring
"""

import json
import asyncio
import logging
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Any
import argparse

try:
    import websockets
    import aiohttp
    from aiohttp import web
except ImportError:
    print("Installing required packages...")
    import subprocess
    subprocess.check_call(['pip', 'install', 'websockets', 'aiohttp'])
    import websockets
    import aiohttp
    from aiohttp import web

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

class BenchmarkDataManager:
    """Manages benchmark data and WebSocket connections"""
    
    def __init__(self):
        self.clients: set = set()
        self.benchmark_data = {
            'algorithms': {
                'BFS': {
                    'baseline_ms': 250,
                    'morpheus_ms': 198,
                    'baseline_std': 15,
                    'morpheus_std': 12,
                    'p_value': 0.0001,
                    'cohens_d': 0.85,
                    'l1_hit_rate': 0.88,
                    'l2_hit_rate': 0.92,
                    'l3_hit_rate': 0.96,
                    'ipc': 2.4,
                    'branch_accuracy': 0.98
                },
                'PageRank': {
                    'baseline_ms': 350,
                    'morpheus_ms': 248,
                    'baseline_std': 20,
                    'morpheus_std': 15,
                    'p_value': 0.00001,
                    'cohens_d': 1.12,
                    'l1_hit_rate': 0.85,
                    'l2_hit_rate': 0.90,
                    'l3_hit_rate': 0.95,
                    'ipc': 2.8,
                    'branch_accuracy': 0.97
                },
                'Betweenness': {
                    'baseline_ms': 420,
                    'morpheus_ms': 365,
                    'baseline_std': 25,
                    'morpheus_std': 20,
                    'p_value': 0.0005,
                    'cohens_d': 0.72,
                    'l1_hit_rate': 0.82,
                    'l2_hit_rate': 0.88,
                    'l3_hit_rate': 0.94,
                    'ipc': 2.1,
                    'branch_accuracy': 0.96
                }
            },
            'timestamp': datetime.now().isoformat()
        }
        self.history: List[Dict[str, Any]] = []
    
    def add_client(self, websocket):
        """Register a new WebSocket client"""
        self.clients.add(websocket)
        logger.info(f"Client connected. Total clients: {len(self.clients)}")
    
    def remove_client(self, websocket):
        """Unregister a WebSocket client"""
        self.clients.discard(websocket)
        logger.info(f"Client disconnected. Total clients: {len(self.clients)}")
    
    async def broadcast_update(self, data: Dict[str, Any]):
        """Broadcast data to all connected clients"""
        if self.clients:
            message = json.dumps(data)
            disconnected = set()
            
            for client in self.clients:
                try:
                    await client.send(message)
                except websockets.exceptions.ConnectionClosed:
                    disconnected.add(client)
            
            # Clean up disconnected clients
            for client in disconnected:
                self.remove_client(client)
    
    def update_benchmark(self, algorithm: str, **kwargs):
        """Update benchmark data for an algorithm"""
        if algorithm in self.benchmark_data['algorithms']:
            self.benchmark_data['algorithms'][algorithm].update(kwargs)
            self.benchmark_data['timestamp'] = datetime.now().isoformat()
            self.history.append({
                'timestamp': datetime.now().isoformat(),
                'algorithm': algorithm,
                'data': self.benchmark_data['algorithms'][algorithm].copy()
            })
            return True
        return False
    
    def get_data(self) -> Dict[str, Any]:
        """Get current benchmark data"""
        return self.benchmark_data.copy()
    
    def get_history(self, limit: int = 100) -> List[Dict[str, Any]]:
        """Get historical data"""
        return self.history[-limit:]

class DashboardServer:
    """Async HTTP/WebSocket server for the dashboard"""
    
    def __init__(self, host: str = 'localhost', port: int = 8080):
        self.host = host
        self.port = port
        self.data_manager = BenchmarkDataManager()
        self.app = web.Application()
        self.setup_routes()
    
    def setup_routes(self):
        """Setup HTTP routes"""
        self.app.router.add_get('/', self.handle_index)
        self.app.router.add_get('/api/data', self.handle_get_data)
        self.app.router.add_post('/api/update', self.handle_update)
        self.app.router.add_get('/api/history', self.handle_history)
        self.app.router.add_websocket_handler('/ws', self.handle_websocket)
        
        # Serve static assets
        static_dir = Path(__file__).parent
        self.app.router.add_static('/', static_dir)
    
    async def handle_index(self, request):
        """Serve the dashboard HTML"""
        dashboard_path = Path(__file__).parent / 'dashboard.html'
        if dashboard_path.exists():
            return web.FileResponse(dashboard_path)
        return web.Response(text='Dashboard HTML not found', status=404)
    
    async def handle_get_data(self, request):
        """API endpoint to get current data"""
        return web.json_response(self.data_manager.get_data())
    
    async def handle_update(self, request):
        """API endpoint to update benchmark data"""
        try:
            data = await request.json()
            algorithm = data.get('algorithm')
            
            if not algorithm:
                return web.json_response(
                    {'error': 'Missing algorithm parameter'},
                    status=400
                )
            
            # Remove algorithm key from update data
            update_data = {k: v for k, v in data.items() if k != 'algorithm'}
            
            if self.data_manager.update_benchmark(algorithm, **update_data):
                # Broadcast update to all connected clients
                await self.data_manager.broadcast_update({
                    'type': 'benchmark_update',
                    'data': self.data_manager.get_data()
                })
                
                return web.json_response({
                    'success': True,
                    'message': f'Updated {algorithm}'
                })
            else:
                return web.json_response(
                    {'error': f'Algorithm {algorithm} not found'},
                    status=404
                )
        except json.JSONDecodeError:
            return web.json_response(
                {'error': 'Invalid JSON'},
                status=400
            )
    
    async def handle_history(self, request):
        """API endpoint to get historical data"""
        limit = int(request.query.get('limit', 100))
        return web.json_response(self.data_manager.get_history(limit))
    
    async def handle_websocket(self, request):
        """WebSocket handler for real-time updates"""
        ws = web.WebSocketResponse()
        await ws.prepare(request)
        
        self.data_manager.add_client(ws)
        
        # Send initial data
        try:
            await ws.send_json({
                'type': 'connection',
                'message': 'Connected to Morpheus Dashboard',
                'data': self.data_manager.get_data()
            })
        except Exception as e:
            logger.error(f"Error sending initial data: {e}")
        
        try:
            async for msg in ws:
                if msg.type == web.WSMsgType.TEXT:
                    try:
                        data = json.loads(msg.data)
                        message_type = data.get('type')
                        
                        if message_type == 'ping':
                            await ws.send_json({'type': 'pong'})
                        elif message_type == 'request_data':
                            await ws.send_json({
                                'type': 'data',
                                'data': self.data_manager.get_data()
                            })
                        elif message_type == 'update':
                            # Handle benchmark update via WebSocket
                            algorithm = data.get('algorithm')
                            update_data = {k: v for k, v in data.items() 
                                         if k not in ['type', 'algorithm']}
                            
                            if self.data_manager.update_benchmark(algorithm, **update_data):
                                await self.data_manager.broadcast_update({
                                    'type': 'benchmark_update',
                                    'data': self.data_manager.get_data()
                                })
                    except json.JSONDecodeError:
                        await ws.send_json({'type': 'error', 'message': 'Invalid JSON'})
                
                elif msg.type == web.WSMsgType.ERROR:
                    logger.error(f'WebSocket error: {ws.exception()}')
        
        finally:
            self.data_manager.remove_client(ws)
        
        return ws
    
    def run(self):
        """Start the server"""
        logger.info(f"Starting Morpheus Dashboard Server on http://{self.host}:{self.port}")
        logger.info(f"Open your browser to http://{self.host}:{self.port}")
        logger.info("Press Ctrl+C to stop")
        
        web.run_app(self.app, host=self.host, port=self.port)

class CLIInterface:
    """Command-line interface for updating benchmarks"""
    
    def __init__(self, server_url: str = 'http://localhost:8080'):
        self.server_url = server_url
    
    async def update_benchmark(self, algorithm: str, **kwargs):
        """Update benchmark via API"""
        try:
            async with aiohttp.ClientSession() as session:
                data = {'algorithm': algorithm, **kwargs}
                async with session.post(
                    f'{self.server_url}/api/update',
                    json=data
                ) as resp:
                    result = await resp.json()
                    return result
        except Exception as e:
            logger.error(f"Failed to update benchmark: {e}")
            return None
    
    async def get_data(self):
        """Get current data from server"""
        try:
            async with aiohttp.ClientSession() as session:
                async with session.get(f'{self.server_url}/api/data') as resp:
                    return await resp.json()
        except Exception as e:
            logger.error(f"Failed to get data: {e}")
            return None

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='Morpheus Benchmark Dashboard Server'
    )
    parser.add_argument(
        '--host',
        default='localhost',
        help='Server host (default: localhost)'
    )
    parser.add_argument(
        '--port',
        type=int,
        default=8080,
        help='Server port (default: 8080)'
    )
    parser.add_argument(
        '--update',
        action='store_true',
        help='Update a benchmark via CLI'
    )
    parser.add_argument(
        '--algorithm',
        help='Algorithm to update'
    )
    parser.add_argument(
        '--morpheus-time',
        type=float,
        help='Morpheus execution time (ms)'
    )
    parser.add_argument(
        '--baseline-time',
        type=float,
        help='Baseline execution time (ms)'
    )
    
    args = parser.parse_args()
    
    if args.update:
        # CLI mode: update benchmark
        if not args.algorithm or not args.morpheus_time:
            print("Error: --algorithm and --morpheus-time required for update")
            return
        
        cli = CLIInterface(f'http://{args.host}:{args.port}')
        
        update_data = {
            'morpheus_ms': args.morpheus_time,
        }
        if args.baseline_time:
            update_data['baseline_ms'] = args.baseline_time
        
        result = asyncio.run(cli.update_benchmark(args.algorithm, **update_data))
        if result:
            print(f"✓ Updated {args.algorithm}: {result['message']}")
        else:
            print(f"✗ Failed to update {args.algorithm}")
    else:
        # Server mode: start dashboard
        server = DashboardServer(host=args.host, port=args.port)
        try:
            server.run()
        except KeyboardInterrupt:
            logger.info("Server stopped")

if __name__ == '__main__':
    main()
